#pragma once

#include "tcpengine/network/Session.hpp"
#include "tcpengine/network/packet/IPacket.hpp"

#include <memory>
#include <string>
#include <utility>

class User
{
public:
	explicit User(std::shared_ptr<tcpengine::Session> session);

	tcpengine::Session::SessionId getId() const { return m_session->getId(); }

	const std::string& getNickname() const { return m_nickname; }
	void setNickname(std::string nickname) { m_nickname = std::move(nickname); }

	bool isEntered() const { return !m_nickname.empty(); }

	void send(const tcpengine::IPacket& packet);

	void send(std::shared_ptr<const tcpengine::Session::Buffer> buffer);

private:
	std::shared_ptr<tcpengine::Session> m_session;
	std::string m_nickname;
};
