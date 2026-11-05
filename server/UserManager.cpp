#include "UserManager.hpp"

std::shared_ptr<User> UserManager::add(const std::shared_ptr<tcpengine::Session>& session)
{
	auto user = std::make_shared<User>(session);
	m_users[user->getId()] = user;
	return user;
}

void UserManager::remove(tcpengine::Session::SessionId id)
{
	m_users.erase(id);
}

std::shared_ptr<User> UserManager::find(tcpengine::Session::SessionId id) const
{
	const auto it = m_users.find(id);
	return it != m_users.end() ? it->second : nullptr;
}

bool UserManager::isNicknameTaken(const std::string& nickname) const
{
	for (const auto& [id, user] : m_users) {
		if (user->isEntered() && user->getNickname() == nickname) {
			return true;
		}
	}
	return false;
}

void UserManager::broadcast(const tcpengine::IPacket& packet, const User* except)
{
	std::shared_ptr<const tcpengine::Session::Buffer> buffer = packet.serialize();
	if (!buffer) {
		return;
	}

	for (const auto& [id, user] : m_users) {
		if (user->isEntered() && user.get() != except) {
			user->send(buffer);
		}
	}
}
