#include "User.hpp"

#include <utility>

User::User(std::shared_ptr<tcpengine::Session> session, std::string nickname, tcpengine::TokenBucket chatLimit)
	: m_session(std::move(session)),
	m_nickname(std::move(nickname)),
	m_chatLimit(chatLimit)
{
}

void User::send(const tcpengine::PacketBase& packet)
{
	m_session->send(packet);
}

void User::send(std::shared_ptr<const tcpengine::PacketBuffer> buffer)
{
	m_session->send(std::move(buffer));
}
