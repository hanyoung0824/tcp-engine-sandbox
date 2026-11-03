#include "UserManager.hpp"

#include "PacketHelper.hpp"

UserManager& UserManager::instance()
{
	// 처음 호출될 때 한 번만 만들어진다 (C++11부터 이 초기화는 스레드 안전하다).
	static UserManager instance;
	return instance;
}

std::shared_ptr<User> UserManager::add(const std::shared_ptr<Session>& session)
{
	auto user = std::make_shared<User>(session);
	m_users[user->getId()] = user;
	return user;
}

void UserManager::remove(Session::SessionId id)
{
	m_users.erase(id);
}

std::shared_ptr<User> UserManager::find(Session::SessionId id) const
{
	const auto it = m_users.find(id);
	return it != m_users.end() ? it->second : nullptr;
}

void UserManager::broadcast(const IPacket& packet)
{
	std::shared_ptr<const PacketBuffer> buffer = makePacket(packet);
	if (!buffer) {
		return;
	}

	// User::send()는 큐에 넣고 바로 반환하며 그 자리에서 목록을 바꾸지 않으므로, 순회하는 동안 목록이 바뀌지 않는다.
	for (const auto& [id, user] : m_users) {
		user->send(buffer);
	}
}
