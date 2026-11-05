#include "packets/ChatPacket.hpp"

#include "ServerToClient.hpp"
#include "Session.hpp"
#include "UserManager.hpp"

#include <iostream>
#include <thread>

void ChatPacket::handle(const std::shared_ptr<Session>& session)
{
	// 패킷을 보낸 연결이 어떤 유저인지 찾는다.
	std::shared_ptr<User> user = UserManager::instance().find(session->getId());
	if (!user) {
		return;
	}

	std::cout << "[thread " << std::this_thread::get_id() << "] " << user->getName() << ": " << message << '\n';

	UserManager::instance().broadcast(S2C_ChatNotify(user->getName(), message));
}
