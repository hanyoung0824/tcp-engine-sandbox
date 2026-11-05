#include "ChatClient.hpp"

#include "ChatState.hpp"
#include "ClientToServer.hpp"
#include "ConsoleLog.hpp"
#include "ServerToClientHandler.hpp"

#include <iostream>
#include <string>

ChatClient::ChatClient()
{
	registerPacket<S2C_ChatNotifyHandler>();
	registerPacket<S2C_SetNicknameResultHandler>();
}

bool ChatClient::onStart()
{
	setConsoleLogHandler(LogLevel::Warning);
	return true;
}

void ChatClient::onRun()
{
	std::cout << "Connected. Enter your nickname (empty line to quit):\n";

	std::string line;
	while (std::getline(std::cin, line) && !line.empty() && isConnected()) {
		if (!ChatState::instance().isEntered()) {
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

void ChatClient::onDisconnected(const std::shared_ptr<Session>&)
{
	if (!m_quitting) {
		std::cout << "Disconnected from server (press Enter to exit)\n" << std::flush;
	}
}
