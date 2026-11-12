#include "ChatClient.hpp"

#include "ClientToServer.hpp"

#include <iostream>
#include <string>

ChatClient::ChatClient(const tcpengine::ClientSettings& settings)
	: Client(settings)
{
	registerHandler<s2c::ChatNotify>([this](const auto&, const s2c::ChatNotify& packet) {
		handleChatNotify(packet);
	});
	registerHandler<s2c::SetNicknameResult>([this](const auto&, const s2c::SetNicknameResult& packet) {
		handleSetNicknameResult(packet);
	});
}

void ChatClient::onRun()
{
	std::cout << "Connected. Enter your nickname (empty line to quit):\n";

	std::string line;
	while (std::getline(std::cin, line) && !line.empty() && isConnected()) {
		// 서버가 같은 validate()로 검사해 틀리면 연결을 끊으므로 보내기 전에 걸러 냄
		if (!m_state.isEntered()) {
			const c2s::SetNickname packet(line);
			if (!packet.validate()) {
				std::cerr << "Nickname too long (max " << maxNicknameLength << " bytes)\n";
				continue;
			}
			send(packet);
			continue;
		}

		const c2s::Chat packet(line);
		if (!packet.validate()) {
			std::cerr << "Message too long (max " << maxChatMessageLength << " bytes)\n";
			continue;
		}
		send(packet);
	}

	m_quitting = true;
}

void ChatClient::onDisconnected(const std::shared_ptr<tcpengine::Connection>&)
{
	if (!m_quitting) {
		std::cout << "Disconnected from server (press Enter to exit)\n" << std::flush;
	}
}

void ChatClient::handleChatNotify(const s2c::ChatNotify& packet)
{
	std::cout << '[' << packet.sender << "] " << packet.message << '\n' << std::flush;
}

void ChatClient::handleSetNicknameResult(const s2c::SetNicknameResult& packet)
{
	switch (packet.result) {
	case NicknameResult::Ok:
		m_state.setNickname(packet.nickname);
		std::cout << "Entered the chat as " << packet.nickname << " (empty line to quit)\n" << std::flush;
		return;
	case NicknameResult::Empty:
		std::cout << "Nickname is empty.";
		break;
	case NicknameResult::Duplicate:
		std::cout << "Nickname is already in use.";
		break;
	case NicknameResult::AlreadySet:
		std::cout << "Nickname is already set.\n" << std::flush;
		return;
	}
	std::cout << " Enter another nickname:\n" << std::flush;
}
