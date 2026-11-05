#include "ChatClient.hpp"

#include "ClientToServer.hpp"

#include <iostream>
#include <string>

ChatClient::ChatClient()
{
	registerHandler<S2C_ChatNotify>([this](const auto&, const S2C_ChatNotify& packet) {
		handleChatNotify(packet);
	});
	registerHandler<S2C_SetNicknameResult>([this](const auto&, const S2C_SetNicknameResult& packet) {
		handleSetNicknameResult(packet);
	});
}

void ChatClient::onRun()
{
	std::cout << "Connected. Enter your nickname (empty line to quit):\n";

	std::string line;
	while (std::getline(std::cin, line) && !line.empty() && isConnected()) {
		if (!m_state.isEntered()) {
			if (line.size() > maxNicknameLength) {
				std::cerr << "Nickname too long (max " << maxNicknameLength << " bytes)\n";
				continue;
			}
			send(C2S_SetNickname(line));
			continue;
		}

		if (line.size() > maxChatMessageLength) {
			std::cerr << "Message too long (max " << maxChatMessageLength << " bytes)\n";
			continue;
		}

		send(C2S_Chat(line));
	}

	m_quitting = true;
}

void ChatClient::onDisconnected(const std::shared_ptr<tcpengine::Session>&)
{
	if (!m_quitting) {
		std::cout << "Disconnected from server (press Enter to exit)\n" << std::flush;
	}
}

void ChatClient::handleChatNotify(const S2C_ChatNotify& packet)
{
	std::cout << '[' << packet.sender << "] " << packet.message << '\n' << std::flush;
}

void ChatClient::handleSetNicknameResult(const S2C_SetNicknameResult& packet)
{
	switch (packet.result) {
	case SetNicknameResult::Ok:
		m_state.setNickname(packet.nickname);
		std::cout << "Entered the chat as " << packet.nickname << " (empty line to quit)\n" << std::flush;
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
