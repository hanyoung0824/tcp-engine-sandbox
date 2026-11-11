#include "ChatServer.hpp"

#include "ServerToClient.hpp"
#include "tcpengine/network/Connection.hpp"
#include "tcpengine/utility/Log.hpp"

ChatServer::ChatServer(const tcpengine::ServerSettings& engineSettings, const ChatServerSettings& settings)
	: Server(engineSettings),
	m_settings(settings)
{
	registerHandler<c2s::Chat>([this](const auto& session, const c2s::Chat& packet) {
		handleChat(session, packet);
	});
	registerConnectionHandler<c2s::SetNickname>([this](const auto& connection, const c2s::SetNickname& packet) {
		handleSetNickname(connection, packet);
	});
}

void ChatServer::onStop()
{
	tcpengine::logInfo("Server stopped");
}

void ChatServer::onSessionResumed(const std::shared_ptr<tcpengine::Session>& session)
{
	if (std::shared_ptr<User> user = m_users.find(session->getId())) {
		tcpengine::logInfo(user->getNickname(), " reconnected");
	}
}

void ChatServer::onSessionDetached(const std::shared_ptr<tcpengine::Session>& session)
{
	if (std::shared_ptr<User> user = m_users.find(session->getId())) {
		tcpengine::logInfo(user->getNickname(), " disconnected, waiting for reconnect");
	}
}

void ChatServer::onSessionEnded(const std::shared_ptr<tcpengine::Session>& session)
{
	std::shared_ptr<User> user = m_users.find(session->getId());
	if (!user) {
		return;
	}

	m_users.remove(user->getId());
	m_users.broadcast(s2c::ChatNotify(m_settings.noticeSender, user->getNickname() + " left"));
}

void ChatServer::handleChat(const std::shared_ptr<tcpengine::Session>& session, const c2s::Chat& packet)
{
	std::shared_ptr<User> user = m_users.find(session->getId());
	if (!user) {
		return;
	}

	// 메시지마다 남기면 부하가 클 때 출력이 병목이 되므로 Debug
	tcpengine::logDebug(user->getNickname(), ": ", packet.message);

	m_users.broadcast(s2c::ChatNotify(user->getNickname(), packet.message));
}

void ChatServer::handleSetNickname(const std::shared_ptr<tcpengine::Connection>& connection, const c2s::SetNickname& packet)
{
	NicknameResult result = NicknameResult::Ok;
	if (findSession(connection)) {
		result = NicknameResult::AlreadySet;
	}
	else if (packet.nickname.empty()) {
		result = NicknameResult::Empty;
	}
	else if (m_users.isNicknameTaken(packet.nickname)) {
		result = NicknameResult::Duplicate;
	}

	if (result != NicknameResult::Ok) {
		connection->send(s2c::SetNicknameResult(result, ""));
		return;
	}

	std::shared_ptr<tcpengine::Session> session = startSession(connection);
	if (!session) {
		return;
	}

	std::shared_ptr<User> user = m_users.add(session, packet.nickname);
	tcpengine::logInfo(connection->getAddress(), " entered as ", packet.nickname);

	user->send(s2c::SetNicknameResult(NicknameResult::Ok, packet.nickname));
	m_users.broadcast(s2c::ChatNotify(m_settings.noticeSender, packet.nickname + " joined"), user.get());
}
