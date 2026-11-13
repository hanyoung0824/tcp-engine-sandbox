#pragma once

#include <boost/asio.hpp>

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class Bot;

struct LoadTestOptions
{
	std::string host = "127.0.0.1";
	unsigned short port = 12345;
	int clients = 100;
	int messages = 50;
	std::chrono::milliseconds interval{ 100 };
	std::chrono::seconds timeout{ 60 };
};

// 봇을 모두 접속, 로그인시키고 동시에 채팅을 시작시킨 뒤 받은 결과를 모아 보고함
// 스레드 하나에서 돌아가므로 잠금 없음
class LoadTest
{
public:
	explicit LoadTest(const LoadTestOptions& options);
	~LoadTest();

	LoadTest(const LoadTest&) = delete;
	LoadTest& operator=(const LoadTest&) = delete;

	// 결과를 출력하고 종료 코드를 반환. 누락, 순서 뒤바뀜, 실패한 봇이 있으면 1
	int run();

	const LoadTestOptions& getOptions() const { return m_options; }

	void onLoggedIn();

	// 다른 봇의 채팅 하나를 받음. inOrder는 같은 봇의 이전 채팅보다 뒤에 보낸 것인지
	void onChatReceived(std::chrono::nanoseconds latency, bool inOrder);

	// 서버가 채팅이 너무 빠르다고 알림. 서버 설정의 채팅 제한이 낮다는 뜻
	void onRateLimited();

	void onBotFailed(const std::string& name, const std::string& reason);

private:
	void startChatting();

	void finish(const std::string& reason);

	int report() const;

private:
	const LoadTestOptions m_options;
	boost::asio::io_context m_ioContext;
	boost::asio::steady_timer m_deadline;
	std::vector<std::shared_ptr<Bot>> m_bots;

	int m_loggedIn = 0;
	int m_failed = 0;
	std::uint64_t m_rateLimited = 0;
	const std::uint64_t m_expected;
	std::uint64_t m_received = 0;
	std::uint64_t m_reordered = 0;
	// 지연 시간(마이크로초). 너무 많으면 앞부분만 보관
	std::vector<std::uint32_t> m_latencies;

	std::chrono::steady_clock::time_point m_startTime;
	std::chrono::steady_clock::time_point m_loggedInTime;
	std::chrono::steady_clock::time_point m_lastReceiveTime;
	enum class Phase
	{
		LoggingIn,
		Chatting,
		Finished,
	};
	Phase m_phase = Phase::LoggingIn;
	std::string m_finishReason;
};
