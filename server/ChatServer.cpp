#include "ChatServer.hpp"

#include "Log.hpp"
#include "ServerToClient.hpp"
#include "Session.hpp"

#include <iostream>
#include <thread>

ChatServer::ChatServer()
{
	registerHandler<C2S_Chat>([this](const auto& session, const C2S_Chat& packet) {
		handleChat(session, packet);
	});
	registerHandler<C2S_SetNickname>([this](const auto& session, const C2S_SetNickname& packet) {
		handleSetNickname(session, packet);
	});
}

void ChatServer::onStop()
{
	std::cout << "Server stopped\n";
}

void ChatServer::onConnected(const std::shared_ptr<tcpengine::Session>& session)
{
	m_users.add(session);
}

void ChatServer::onDisconnected(const std::shared_ptr<tcpengine::Session>& session)
{
	std::shared_ptr<User> user = m_users.find(session->getId());
	if (!user) {
		return;
	}

	m_users.remove(user->getId());
	if (user->isEntered()) {
		m_users.broadcast(S2C_ChatNotify("Server", user->getNickname() + " left"));
	}
}

void ChatServer::handleChat(const std::shared_ptr<tcpengine::Session>& session, const C2S_Chat& packet)
{
	std::shared_ptr<User> user = m_users.find(session->getId());
	if (!user) {
		return;
	}

	// 정상 클라이언트는 입장 전에 채팅을 보내지 않음. 연결은 유지하고 패킷만 버림
	if (!user->isEntered()) {
		tcpengine::writeLog(tcpengine::LogLevel::Warning, "Chat before setting a nickname from ", session->getAddress(), ", ignored");
		return;
	}

	std::cout << "[thread " << std::this_thread::get_id() << "] " << user->getNickname() << ": " << packet.message << '\n';

	m_users.broadcast(S2C_ChatNotify(user->getNickname(), packet.message));
}

void ChatServer::handleSetNickname(const std::shared_ptr<tcpengine::Session>& session, const C2S_SetNickname& packet)
{
	std::shared_ptr<User> user = m_users.find(session->getId());
	if (!user) {
		return;
	}

	SetNicknameResult result = SetNicknameResult::Ok;
	if (user->isEntered()) {
		result = SetNicknameResult::AlreadySet;
	}
	else if (packet.nickname.empty()) {
		result = SetNicknameResult::Empty;
	}
	else if (m_users.isNicknameTaken(packet.nickname)) {
		result = SetNicknameResult::Duplicate;
	}

	if (result != SetNicknameResult::Ok) {
		user->send(S2C_SetNicknameResult(result, ""));
		return;
	}

	user->setNickname(packet.nickname);
	std::cout << session->getAddress() << " entered as " << packet.nickname << '\n';

	user->send(S2C_SetNicknameResult(SetNicknameResult::Ok, packet.nickname));
	m_users.broadcast(S2C_ChatNotify("Server", packet.nickname + " joined"), user.get());
}
