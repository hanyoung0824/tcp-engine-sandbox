#pragma once

#include "IPacket.hpp"
#include "PacketHeader.hpp"
#include "Session.hpp"

#include <memory>
#include <string>
#include <utility>

class User
{
public:
	explicit User(std::shared_ptr<Session> session);

	Session::SessionId getId() const { return m_session->getId(); }

	const std::string& getNickname() const { return m_nickname; }
	void setNickname(std::string nickname) { m_nickname = std::move(nickname); }

	bool isEntered() const { return !m_nickname.empty(); }

	void send(const IPacket& packet);

	void send(std::shared_ptr<const PacketBuffer> buffer);

private:
	std::shared_ptr<Session> m_session;
	std::string m_nickname;
};
