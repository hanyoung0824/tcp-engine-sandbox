#include "Bot.hpp"

#include "ClientToServer.hpp"
#include "LoadTest.hpp"
#include "ServerToClient.hpp"
#include "tcpengine/network/packet/PacketWriter.hpp"

#include <algorithm>
#include <charconv>
#include <iomanip>
#include <sstream>
#include <string_view>

namespace asio = boost::asio;
using asio::ip::tcp;
using Clock = std::chrono::steady_clock;

namespace
{
	// 엔진 제어 패킷 (엔진 PacketHeader.hpp와 맞춤)
	constexpr std::uint16_t controlPacketId = 0;
	constexpr std::uint16_t serverHelloType = 1;
	constexpr std::uint16_t pingType = 2;
	constexpr std::uint16_t clientHelloType = 8;
	constexpr std::uint16_t engineProtocolVersion = 1;

	constexpr std::size_t headerSize = 4;
	constexpr std::size_t readBufferSize = 64 * 1024;
	constexpr std::string_view botPrefix = "bot";

	std::string makeBotName(int index)
	{
		std::ostringstream name;
		name << botPrefix << std::setw(4) << std::setfill('0') << index;
		return name.str();
	}

	// 접속 직후 가장 먼저 보내는 버전 알림
	std::shared_ptr<const tcpengine::PacketBuffer> makeClientHelloPacket()
	{
		tcpengine::PacketWriter writer(controlPacketId);
		writer.writeUInt16(clientHelloType);
		writer.writeUInt16(engineProtocolVersion);
		writer.writeUInt16(protocolVersion);
		return std::make_shared<const tcpengine::PacketBuffer>(*writer.finish());
	}

	std::shared_ptr<const tcpengine::PacketBuffer> makePingPacket()
	{
		tcpengine::PacketWriter writer(controlPacketId);
		writer.writeUInt16(pingType);
		return std::make_shared<const tcpengine::PacketBuffer>(*writer.finish());
	}

	// 채팅 본문: "<번호> <보낸 시각(steady_clock 나노초)>"
	std::string makeChatMessage(int seq)
	{
		const auto sentAt = std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now().time_since_epoch()).count();
		return std::to_string(seq) + ' ' + std::to_string(sentAt);
	}

	bool parseInt(std::string_view text, auto& out)
	{
		const auto [end, ec] = std::from_chars(text.data(), text.data() + text.size(), out);
		return ec == std::errc() && end == text.data() + text.size();
	}
}

Bot::Bot(LoadTest& test, asio::io_context& ioContext, int index)
	: m_test(test),
	m_socket(ioContext),
	m_chatTimer(ioContext),
	m_pingTimer(ioContext),
	m_name(makeBotName(index)),
	m_readBuffer(readBufferSize),
	m_lastSeqBySender(test.getOptions().clients, -1)
{
}

void Bot::connect(const tcp::endpoint& endPoint)
{
	m_socket.async_connect(endPoint, [self = shared_from_this()](const boost::system::error_code& ec) {
		if (self->m_state == State::Closed) {
			return;
		}
		if (ec) {
			self->fail("connect: " + ec.message());
			return;
		}
		self->m_state = State::Connected;

		boost::system::error_code ignored;
		self->m_socket.set_option(tcp::no_delay(true), ignored);

		self->send(makeClientHelloPacket());
		self->send(c2s::SetNickname(self->m_name).serialize());
		self->startRead();
	});
}

void Bot::startChatting(std::chrono::milliseconds firstDelay)
{
	m_nextChatTime = Clock::now() + firstDelay;
	scheduleChat();
}

void Bot::shutdown()
{
	if (m_state != State::Connected) {
		close();
		return;
	}
	// 받지 않은 데이터가 남은 채 닫으면 RST가 나가 서버에 읽기 오류로 남음. FIN을 보내고 서버가 닫을 때까지 받아서 버림
	m_state = State::Closing;
	m_chatTimer.cancel();
	m_pingTimer.cancel();
	boost::system::error_code ignored;
	m_socket.shutdown(tcp::socket::shutdown_send, ignored);
}

void Bot::close()
{
	if (m_state == State::Closed) {
		return;
	}
	m_state = State::Closed;
	m_chatTimer.cancel();
	m_pingTimer.cancel();
	boost::system::error_code ignored;
	m_socket.close(ignored);
	m_test.onBotClosed();
}

void Bot::send(std::shared_ptr<const tcpengine::PacketBuffer> data)
{
	if (m_state != State::Connected || !data) {
		return;
	}
	const bool writing = !m_writeQueue.empty();
	m_writeQueue.push_back(std::move(data));
	if (!writing) {
		startWrite();
	}
}

void Bot::startWrite()
{
	asio::async_write(m_socket, asio::buffer(*m_writeQueue.front()),
		[self = shared_from_this()](const boost::system::error_code& ec, std::size_t) {
			if (self->m_state == State::Closed) {
				return;
			}
			if (ec) {
				self->fail("write: " + ec.message());
				return;
			}
			self->m_writeQueue.pop_front();
			if (!self->m_writeQueue.empty()) {
				self->startWrite();
			}
		});
}

void Bot::startRead()
{
	asio::mutable_buffer freeSpace = asio::buffer(m_readBuffer) + m_readLength;
	m_socket.async_read_some(freeSpace, [self = shared_from_this()](const boost::system::error_code& ec, std::size_t length) {
		self->onRead(ec, length);
	});
}

void Bot::onRead(const boost::system::error_code& ec, std::size_t length)
{
	if (m_state == State::Closed) {
		return;
	}
	if (m_state == State::Closing) {
		if (ec) {
			close();
			return;
		}
		m_readLength = 0;
		startRead();
		return;
	}
	if (ec) {
		fail(ec == asio::error::eof ? "closed by server" : "read: " + ec.message());
		return;
	}

	m_readLength += length;
	const std::span<const std::uint8_t> received(m_readBuffer.data(), m_readLength);

	std::size_t processed = 0;
	while (received.size() - processed >= headerSize) {
		const std::span<const std::uint8_t> rest = received.subspan(processed);
		tcpengine::PacketReader header(rest.first(headerSize));
		std::uint16_t size = 0;
		std::uint16_t id = 0;
		header.readUInt16(size);
		header.readUInt16(id);

		if (size < headerSize || size > tcpengine::maxPacketSize) {
			fail("invalid packet size " + std::to_string(size));
			return;
		}
		if (rest.size() < size) {
			break;
		}

		tcpengine::PacketReader body(rest.subspan(headerSize, size - headerSize));
		if (!handlePacket(id, body)) {
			fail("invalid packet id " + std::to_string(id));
			return;
		}
		if (m_state == State::Closed) {
			return;
		}
		processed += size;
	}

	std::copy(m_readBuffer.begin() + processed, m_readBuffer.begin() + m_readLength, m_readBuffer.begin());
	m_readLength -= processed;
	startRead();
}

bool Bot::handlePacket(std::uint16_t id, tcpengine::PacketReader& body)
{
	if (id == controlPacketId) {
		return handleControl(body);
	}

	if (id == s2c::ChatNotify::id) {
		s2c::ChatNotify notify;
		if (!notify.read(body) || !notify.validate()) {
			return false;
		}
		handleChatNotify(notify);
		return true;
	}

	if (id == s2c::SetNicknameResult::id) {
		s2c::SetNicknameResult result;
		if (!result.read(body) || !result.validate()) {
			return false;
		}
		if (result.result != NicknameResult::Ok) {
			fail("nickname rejected");
			return true;
		}
		m_test.onLoggedIn();
		return true;
	}

	return false;
}

bool Bot::handleControl(tcpengine::PacketReader& body)
{
	std::uint16_t type = 0;
	if (!body.readUInt16(type)) {
		return false;
	}

	// ServerHello로 받은 간격마다 Ping을 보냄. 나머지 제어 패킷(Pong, 세션)은 무시
	if (type == serverHelloType) {
		std::uint16_t interval = 0;
		std::uint16_t timeout = 0;
		if (!body.readUInt16(interval) || !body.readUInt16(timeout)) {
			return false;
		}
		m_pingInterval = std::chrono::seconds(interval);
		schedulePing();
	}
	return true;
}

void Bot::handleChatNotify(const s2c::ChatNotify& notify)
{
	const std::string_view sender = notify.sender;
	if (!sender.starts_with(botPrefix)) {
		if (notify.message.find("too fast") != std::string::npos) {
			m_test.onRateLimited();
		}
		return;
	}

	int senderIndex = 0;
	const std::string_view message = notify.message;
	const std::size_t space = message.find(' ');
	int seq = 0;
	std::int64_t sentAt = 0;
	if (!parseInt(sender.substr(botPrefix.size()), senderIndex)
		|| senderIndex < 0 || senderIndex >= static_cast<int>(m_lastSeqBySender.size())
		|| space == std::string_view::npos
		|| !parseInt(message.substr(0, space), seq)
		|| !parseInt(message.substr(space + 1), sentAt)) {
		return;
	}

	int& lastSeq = m_lastSeqBySender[senderIndex];
	const bool inOrder = seq > lastSeq;
	lastSeq = std::max(lastSeq, seq);

	const auto now = std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now().time_since_epoch());
	m_test.onChatReceived(now - std::chrono::nanoseconds(sentAt), inOrder);
}

void Bot::scheduleChat()
{
	if (m_sentChats >= m_test.getOptions().messages) {
		return;
	}
	m_chatTimer.expires_at(m_nextChatTime);
	m_chatTimer.async_wait([self = shared_from_this()](const boost::system::error_code& ec) {
		if (ec || self->m_state != State::Connected) {
			return;
		}
		self->send(c2s::Chat(makeChatMessage(self->m_sentChats)).serialize());
		++self->m_sentChats;
		self->m_nextChatTime += self->m_test.getOptions().interval;
		self->scheduleChat();
	});
}

void Bot::schedulePing()
{
	if (m_pingInterval.count() == 0) {
		return;
	}
	m_pingTimer.expires_after(m_pingInterval);
	m_pingTimer.async_wait([self = shared_from_this()](const boost::system::error_code& ec) {
		if (ec || self->m_state != State::Connected) {
			return;
		}
		self->send(makePingPacket());
		self->schedulePing();
	});
}

void Bot::fail(const std::string& reason)
{
	if (m_state == State::Closed) {
		return;
	}
	// 끝내는 중의 오류는 결과와 상관없으므로 알리지 않음
	const bool closing = m_state == State::Closing;
	close();
	if (!closing) {
		m_test.onBotFailed(m_name, reason);
	}
}
