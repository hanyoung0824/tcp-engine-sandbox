#include "ClientToServerHandler.hpp"

#include "Log.hpp"
#include "ServerToClient.hpp"
#include "Session.hpp"
#include "UserManager.hpp"

#include <iostream>
#include <thread>

void C2S_ChatHandler::handle(const std::shared_ptr<Session>& session)
{
	std::shared_ptr<User> user = UserManager::instance().find(session->getId());
	if (!user) {
		return;
	}

	// 정상 클라이언트는 입장 전에 채팅을 보내지 않는다. 연결은 유지하고 패킷만 버린다.
	if (!user->isEntered()) {
		writeLog(LogLevel::Warning, "Chat before setting a nickname from ", session->getAddress(), ", ignored");
		return;
	}

	std::cout << "[thread " << std::this_thread::get_id() << "] " << user->getNickname() << ": " << message << '\n';

	UserManager::instance().broadcast(S2C_ChatNotify(user->getNickname(), message));
}

void C2S_SetNicknameHandler::handle(const std::shared_ptr<Session>& session)
{
	std::shared_ptr<User> user = UserManager::instance().find(session->getId());
	if (!user) {
		return;
	}

	SetNicknameResult result = SetNicknameResult::Ok;
	if (user->isEntered()) {
		result = SetNicknameResult::AlreadySet;
	}
	else if (nickname.empty()) {
		result = SetNicknameResult::Empty;
	}
	else if (UserManager::instance().isNicknameTaken(nickname)) {
		result = SetNicknameResult::Duplicate;
	}

	if (result != SetNicknameResult::Ok) {
		user->send(S2C_SetNicknameResult(result, ""));
		return;
	}

	user->setNickname(nickname);
	std::cout << session->getAddress() << " entered as " << nickname << '\n';

	user->send(S2C_SetNicknameResult(SetNicknameResult::Ok, nickname));
	UserManager::instance().broadcast(S2C_ChatNotify("Server", nickname + " joined"), user.get());
}
