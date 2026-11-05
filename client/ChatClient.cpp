#include "ChatClient.hpp"

#include "ClientToServer.hpp"
#include "ConsoleLog.hpp"
#include "packets/S2C_ChatNotifyHandler.hpp"

#include <iostream>
#include <string>

ChatClient::ChatClient()
{
	registerPacket<S2C_ChatNotifyHandler>();
}

bool ChatClient::onStart()
{
	setConsoleLogHandler(LogLevel::Warning);
	return true;
}

void ChatClient::onRun()
{
	std::cout << "Connected (empty line to quit)\n";

	std::string line;
	while (std::getline(std::cin, line) && !line.empty() && isConnected()) {
		// 서버도 길이를 검사해 넘으면 끊으므로, 보내기 전에 먼저 거른다.
		if (line.size() > maxChatMessageLength) {
			std::cerr << "Message too long (max " << maxChatMessageLength << " bytes)\n";
			continue;
		}

		send(C2S_Chat(line));
	}

	// 이후의 종료는 내가 끝낸 것이므로 onDisconnected에서 안내하지 않는다.
	m_quitting = true;
}

void ChatClient::onDisconnected(const std::shared_ptr<Session>&)
{
	if (!m_quitting) {
		std::cout << "Disconnected from server (press Enter to exit)\n" << std::flush;
	}
}
