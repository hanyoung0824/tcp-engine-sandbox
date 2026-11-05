#include "ServerToClientHandler.hpp"

#include "ChatState.hpp"

#include <iostream>

void S2C_ChatNotifyHandler::handle(const std::shared_ptr<Session>&)
{
	std::cout << '[' << sender << "] " << message << '\n' << std::flush;
}

void S2C_SetNicknameResultHandler::handle(const std::shared_ptr<Session>&)
{
	switch (result) {
	case SetNicknameResult::Ok:
		ChatState::instance().setNickname(nickname);
		std::cout << "Entered the chat as " << nickname << " (empty line to quit)\n" << std::flush;
		return;
	case SetNicknameResult::Empty:
		std::cout << "Nickname is empty.";
		break;
	case SetNicknameResult::Duplicate:
		std::cout << "Nickname is already in use.";
		break;
	case SetNicknameResult::AlreadySet:
		std::cout << "Nickname is already set.\n" << std::flush;
		return;
	}
	std::cout << " Enter another nickname:\n" << std::flush;
}
