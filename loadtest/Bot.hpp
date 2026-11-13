#pragma once

#include "tcpengine/network/packet/Packet.hpp"
#include "tcpengine/network/packet/PacketReader.hpp"

#include <boost/asio.hpp>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <memory>
#include <span>
#include <string>
#include <vector>

class LoadTest;

namespace s2c
{
class ChatNotify;
}

// 채팅 클라이언트 하나를 흉내 냄. 엔진 Client는 인스턴스마다 스레드를 띄우므로 쓰지 않고
// 소켓 하나로 프로토콜을 직접 주고받음. 하트비트는 서버가 알려 준 간격으로 Ping만 보냄
class Bot : public std::enable_shared_from_this<Bot>
{
public:
	Bot(LoadTest& test, boost::asio::io_context& ioContext, int index);

	void connect(const boost::asio::ip::tcp::endpoint& endPoint);

	// firstDelay 뒤부터 interval마다 채팅을 보냄. 봇마다 시작을 어긋나게 해서 한꺼번에 몰리지 않게 함
	void startChatting(std::chrono::milliseconds firstDelay);

	// 서버에 FIN을 보내고 서버가 닫기를 기다림. 그동안 받은 것은 버림
	void shutdown();

	void close();

	const std::string& getName() const { return m_name; }

private:
	enum class State
	{
		Connecting,
		Connected,
		Closing,
		Closed,
	};

	void send(std::shared_ptr<const tcpengine::PacketBuffer> data);

	void startWrite();

	void startRead();

	void onRead(const boost::system::error_code& ec, std::size_t length);

	// 패킷 하나를 처리. 형식이 틀리면 false
	bool handlePacket(std::uint16_t id, tcpengine::PacketReader& body);

	bool handleControl(tcpengine::PacketReader& body);

	void handleChatNotify(const s2c::ChatNotify& notify);

	void scheduleChat();

	void schedulePing();

	void fail(const std::string& reason);

private:
	LoadTest& m_test;
	boost::asio::ip::tcp::socket m_socket;
	boost::asio::steady_timer m_chatTimer;
	boost::asio::steady_timer m_pingTimer;
	const std::string m_name;
	State m_state = State::Connecting;

	int m_sentChats = 0;
	std::chrono::steady_clock::time_point m_nextChatTime;
	std::chrono::seconds m_pingInterval{ 0 };

	std::vector<std::uint8_t> m_readBuffer;
	std::size_t m_readLength = 0;
	std::deque<std::shared_ptr<const tcpengine::PacketBuffer>> m_writeQueue;

	// 보낸 봇마다 마지막으로 받은 채팅 번호. 순서 검사용
	std::vector<int> m_lastSeqBySender;
};
