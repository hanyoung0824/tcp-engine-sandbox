#include "ChatClient.hpp"

#include "ClientToServer.hpp"

#include <iostream>
#include <iterator>
#include <string>
#include <utility>

ChatClient::ChatClient(const tcpengine::ClientSettings& settings)
	: Client(settings, protocolVersion)
{
	registerHandler<s2c::ChatNotify>([this](const s2c::ChatNotify& packet) {
		handleChatNotify(packet);
	});
	registerHandler<s2c::SetNicknameResult>([this](const s2c::SetNicknameResult& packet) {
		handleSetNicknameResult(packet);
	});
}

ChatClient::~ChatClient()
{
	// 빈 줄이나 입력 끝을 받아 quit()한 뒤라 입력 스레드는 이미 끝남
	if (m_inputThread.joinable()) {
		m_inputThread.join();
	}
}

void ChatClient::onUpdate()
{
	for (const std::string& line : takeInputLines()) {
		if (line.empty()) {
			m_quitting = true;
			quit();
			return;
		}
		handleInputLine(line);
	}
}

void ChatClient::onConnected()
{
	if (m_inputThread.joinable()) {
		return;
	}
	std::cout << "Connected. Enter your nickname (empty line to quit):\n" << std::flush;
	m_inputThread = std::thread([this] { readInput(); });
}

void ChatClient::readInput()
{
	std::string line;
	while (std::getline(std::cin, line)) {
		const bool done = line.empty();
		{
			const std::lock_guard lock(m_inputMutex);
			m_inputLines.push_back(std::move(line));
		}
		if (done) {
			return;
		}
	}
	const std::lock_guard lock(m_inputMutex);
	m_inputLines.emplace_back();
}

std::vector<std::string> ChatClient::takeInputLines()
{
	const std::lock_guard lock(m_inputMutex);
	std::vector<std::string> lines(std::make_move_iterator(m_inputLines.begin()), std::make_move_iterator(m_inputLines.end()));
	m_inputLines.clear();
	return lines;
}

void ChatClient::handleInputLine(const std::string& line)
{
	if (!isConnected()) {
		std::cerr << "Not connected\n";
		return;
	}

	// 서버가 같은 validate()로 검사해 틀리면 연결을 끊으므로 보내기 전에 걸러 냄
	if (!m_state.isEntered()) {
		const c2s::SetNickname packet(line);
		if (!packet.validate()) {
			std::cerr << "Nickname too long (max " << maxNicknameLength << " bytes)\n";
			return;
		}
		send(packet);
		return;
	}

	const c2s::Chat packet(line);
	if (!packet.validate()) {
		std::cerr << "Message too long (max " << maxChatMessageLength << " bytes)\n";
		return;
	}
	send(packet);
}

void ChatClient::onDisconnected(const tcpengine::DisconnectInfo& info)
{
	if (m_quitting) {
		return;
	}
	std::cout << "Disconnected from server";
	if (info.reason) {
		std::cout << " (" << tcpengine::toString(*info.reason) << ')';
	}
	std::cout << (info.willReconnect ? ", reconnecting...\n" : ", press Enter to exit\n") << std::flush;
}

void ChatClient::onSessionResumed()
{
	std::cout << "Reconnected\n" << std::flush;
}

void ChatClient::onSessionLost()
{
	m_state.clear();
	std::cout << "Reconnected, but the session has ended. Enter your nickname:\n" << std::flush;
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
