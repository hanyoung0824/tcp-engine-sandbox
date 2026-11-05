#include "User.hpp"

#include "PacketHelper.hpp"

User::User(std::shared_ptr<Session> session)
	: m_session(std::move(session))
{
}

void User::send(const IPacket& packet)
{
	sendPacket(*m_session, packet);
}

void User::send(std::shared_ptr<const PacketBuffer> buffer)
{
	m_session->send(std::move(buffer));
}
