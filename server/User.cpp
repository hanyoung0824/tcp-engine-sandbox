#include "User.hpp"

User::User(std::shared_ptr<tcpengine::Session> session)
	: m_session(std::move(session))
{
}

void User::send(const tcpengine::IPacket& packet)
{
	if (auto buffer = packet.serialize()) {
		m_session->send(std::move(buffer));
	}
}

void User::send(std::shared_ptr<const tcpengine::Session::Buffer> buffer)
{
	m_session->send(std::move(buffer));
}
