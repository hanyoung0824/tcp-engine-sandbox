#pragma once

#include "tcpengine/network/packet/Packet.hpp"
#include "tcpengine/server/Session.hpp"
#include "tcpengine/utility/TokenBucket.hpp"

#include <memory>
#include <string>

// 닉네임을 정해 입장한 사용자. 세션에 묶이므로 재접속해도 유지됨
class User
{
public:
	User(std::shared_ptr<tcpengine::Session> session, std::string nickname, tcpengine::TokenBucket chatLimit);

	tcpengine::Session::SessionId getId() const { return m_session->getId(); }

	const std::string& getNickname() const { return m_nickname; }

	void send(const tcpengine::PacketBase& packet);

	void send(std::shared_ptr<const tcpengine::PacketBuffer> buffer);

	// 채팅 한 번을 보내도 되는지. 너무 빠르면 false
	bool tryChat() { return m_chatLimit.tryTake(); }

private:
	std::shared_ptr<tcpengine::Session> m_session;
	const std::string m_nickname;
	tcpengine::TokenBucket m_chatLimit;
};
