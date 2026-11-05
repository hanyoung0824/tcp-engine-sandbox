#include "ChatServer.hpp"

#include "Log.hpp"
#include "ServerToClient.hpp"
#include "Session.hpp"

#include <iostream>
#include <thread>

ChatServer::ChatServer()
{
	registerHandler<c2s::Chat>([this](const auto& session, const c2s::Chat& packet) {
		handleChat(session, packet);
	});
	registerHandler<c2s::SetNickname>([this](const auto& session, const c2s::SetNickname& packet) {
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
		m_users.broadcast(s2c::ChatNotify("Server", user->getNickname() + " left"));
	}
}

void ChatServer::handleChat(const std::shared_ptr<tcpengine::Session>& session, const c2s::Chat& packet)
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

	m_users.broadcast(s2c::ChatNotify(user->getNickname(), packet.message));
}

void ChatServer::handleSetNickname(const std::shared_ptr<tcpengine::Session>& session, const c2s::SetNickname& packet)
{
	std::shared_ptr<User> user = m_users.find(session->getId());
	if (!user) {
		return;
	}

	NicknameResult result = NicknameResult::Ok;
	if (user->isEntered()) {
		result = NicknameResult::AlreadySet;
	}
	else if (packet.nickname.empty()) {
		result = NicknameResult::Empty;
	}
	else if (m_users.isNicknameTaken(packet.nickname)) {
		result = NicknameResult::Duplicate;
	}

	if (result != NicknameResult::Ok) {
		user->send(s2c::SetNicknameResult(result, ""));
		return;
	}

	user->setNickname(packet.nickname);
	std::cout << session->getAddress() << " entered as " << packet.nickname << '\n';

	user->send(s2c::SetNicknameResult(NicknameResult::Ok, packet.nickname));
	m_users.broadcast(s2c::ChatNotify("Server", packet.nickname + " joined"), user.get());
}
