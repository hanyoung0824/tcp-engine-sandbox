#pragma once

#include "IPacket.hpp"
#include "Session.hpp"
#include "User.hpp"

#include <memory>
#include <string>
#include <unordered_map>

// io_context 스레드 하나에서만 쓰므로 잠금이 없다.
class UserManager
{
public:
	static UserManager& instance();

	UserManager(const UserManager&) = delete;
	UserManager& operator=(const UserManager&) = delete;

	std::shared_ptr<User> add(const std::shared_ptr<Session>& session);

	void remove(Session::SessionId id);

	std::shared_ptr<User> find(Session::SessionId id) const;

	bool isNicknameTaken(const std::string& nickname) const;

	void broadcast(const IPacket& packet, const User* except = nullptr);

	std::size_t size() const { return m_users.size(); }

private:
	UserManager() = default;

private:
	std::unordered_map<Session::SessionId, std::shared_ptr<User>> m_users;
};
