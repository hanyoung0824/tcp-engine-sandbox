#pragma once

#include "User.hpp"
#include "tcpengine/network/packet/Packet.hpp"
#include "tcpengine/server/Session.hpp"

#include <memory>
#include <string>
#include <unordered_map>

// io_context 스레드 하나에서만 쓰므로 잠금 없음
class UserManager
{
public:
	UserManager() = default;

	UserManager(const UserManager&) = delete;
	UserManager& operator=(const UserManager&) = delete;

	std::shared_ptr<User> add(const std::shared_ptr<tcpengine::Session>& session, std::string nickname);

	void remove(tcpengine::Session::SessionId id);

	std::shared_ptr<User> find(tcpengine::Session::SessionId id) const;

	bool isNicknameTaken(const std::string& nickname) const;

	void broadcast(const tcpengine::PacketBase& packet, const User* except = nullptr);

private:
	std::unordered_map<tcpengine::Session::SessionId, std::shared_ptr<User>> m_users;
};
