#include "LoadTest.hpp"

#include "Bot.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>

namespace asio = boost::asio;
using asio::ip::tcp;
using Clock = std::chrono::steady_clock;

namespace
{
	// 지연 시간 표본 상한 (4바이트씩이라 약 80MB)
	constexpr std::size_t maxLatencySamples = 20'000'000;

	double toMilliseconds(Clock::duration duration)
	{
		return std::chrono::duration<double, std::milli>(duration).count();
	}
}

LoadTest::LoadTest(const LoadTestOptions& options)
	: m_options(options),
	m_deadline(m_ioContext),
	m_expected(static_cast<std::uint64_t>(options.clients) * options.clients * options.messages)
{
	m_latencies.reserve(static_cast<std::size_t>(std::min<std::uint64_t>(m_expected, maxLatencySamples)));
}

LoadTest::~LoadTest() = default;

int LoadTest::run()
{
	boost::system::error_code ec;
	const asio::ip::address address = asio::ip::make_address(m_options.host, ec);
	if (ec) {
		std::cerr << "invalid host " << m_options.host << ": " << ec.message() << '\n';
		return 2;
	}
	const tcp::endpoint endPoint(address, m_options.port);

	std::cout << "clients " << m_options.clients << ", " << m_options.messages << " chats each every "
		<< m_options.interval.count() << "ms, expecting " << m_expected << " deliveries\n" << std::flush;

	m_startTime = Clock::now();
	for (int i = 0; i < m_options.clients; ++i) {
		auto bot = std::make_shared<Bot>(*this, m_ioContext, i);
		bot->connect(endPoint);
		m_bots.push_back(std::move(bot));
	}

	m_deadline.expires_after(m_options.timeout);
	m_deadline.async_wait([this](const boost::system::error_code& waitError) {
		if (!waitError) {
			finish("timed out");
		}
	});

	m_ioContext.run();
	return report();
}

void LoadTest::onLoggedIn()
{
	++m_loggedIn;
	if (m_loggedIn == m_options.clients) {
		startChatting();
	}
}

void LoadTest::onChatReceived(std::chrono::nanoseconds latency, bool inOrder)
{
	if (m_phase != Phase::Chatting) {
		return;
	}

	++m_received;
	if (!inOrder) {
		++m_reordered;
	}
	if (m_latencies.size() < maxLatencySamples) {
		const auto micros = std::chrono::duration_cast<std::chrono::microseconds>(latency).count();
		m_latencies.push_back(static_cast<std::uint32_t>(std::max<std::int64_t>(micros, 0)));
	}
	m_lastReceiveTime = Clock::now();

	if (m_received == m_expected) {
		finish("completed");
	}
}

void LoadTest::onRateLimited()
{
	++m_rateLimited;
}

void LoadTest::onBotFailed(const std::string& name, const std::string& reason)
{
	if (m_phase == Phase::Finished) {
		return;
	}
	++m_failed;
	// 실패가 많을 때 화면을 덮지 않도록 앞의 몇 개만 출력
	if (m_failed <= 5) {
		std::cerr << name << " failed: " << reason << '\n';
	}
	if (m_failed == m_options.clients) {
		finish("all bots failed");
	}
}

void LoadTest::startChatting()
{
	m_phase = Phase::Chatting;
	m_loggedInTime = Clock::now();
	m_lastReceiveTime = m_loggedInTime;
	std::cout << "all logged in after " << std::fixed << std::setprecision(1)
		<< toMilliseconds(m_loggedInTime - m_startTime) << "ms, chatting\n" << std::flush;

	const auto clients = static_cast<long long>(m_bots.size());
	for (long long i = 0; i < clients; ++i) {
		m_bots[static_cast<std::size_t>(i)]->startChatting(m_options.interval * i / clients);
	}
}

void LoadTest::finish(const std::string& reason)
{
	if (m_phase == Phase::Finished) {
		return;
	}
	m_phase = Phase::Finished;
	m_finishReason = reason;
	m_deadline.cancel();
	for (const std::shared_ptr<Bot>& bot : m_bots) {
		bot->close();
	}
}

int LoadTest::report() const
{
	std::vector<std::uint32_t> latencies = m_latencies;
	std::sort(latencies.begin(), latencies.end());
	const auto percentile = [&latencies](double ratio) {
		if (latencies.empty()) {
			return 0.0;
		}
		const auto index = static_cast<std::size_t>(ratio * static_cast<double>(latencies.size() - 1));
		return latencies[index] / 1000.0;
	};

	const std::uint64_t missing = m_expected - std::min(m_received, m_expected);
	const double seconds = std::chrono::duration<double>(m_lastReceiveTime - m_loggedInTime).count();

	std::cout << std::fixed << std::setprecision(2)
		<< "finished: " << m_finishReason << '\n'
		<< "logged in: " << m_loggedIn << '/' << m_options.clients << ", failed bots: " << m_failed << '\n'
		<< "deliveries: " << m_received << '/' << m_expected
		<< " (missing " << missing << ", reordered " << m_reordered << ", rate limited " << m_rateLimited << ")\n";
	if (seconds > 0) {
		std::cout << "throughput: " << static_cast<double>(m_received) / seconds << " deliveries/s over " << seconds << "s\n";
	}
	std::cout << "latency ms: p50 " << percentile(0.50) << ", p99 " << percentile(0.99)
		<< ", max " << percentile(1.0) << '\n';

	const bool ok = m_failed == 0 && missing == 0 && m_reordered == 0 && m_rateLimited == 0;
	std::cout << "result: " << (ok ? "OK" : "FAIL") << '\n';
	return ok ? 0 : 1;
}
