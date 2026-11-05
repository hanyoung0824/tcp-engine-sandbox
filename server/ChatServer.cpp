#include "ChatServer.hpp"

#include "ConsoleLog.hpp"
#include "ServerToClient.hpp"
#include "Session.hpp"
#include "UserManager.hpp"
#include "packets/C2S_ChatHandler.hpp"

#include <iostream>

ChatServer::ChatServer()
{
	registerPacket<C2S_ChatHandler>();
}

bool ChatServer::onStart()
{
	setConsoleLogHandler(LogLevel::Info);
	return true;
}

void ChatServer::onStop()
{
	std::cout << "Server stopped\n";
}

void ChatServer::onConnected(const std::shared_ptr<Session>& session)
{
	std::shared_ptr<User> user = UserManager::instance().add(session);
	UserManager::instance().broadcast(S2C_ChatNotify("Server", user->getName() + " joined"));
}

void ChatServer::onDisconnected(const std::shared_ptr<Session>& session)
{
	std::shared_ptr<User> user = UserManager::instance().find(session->getId());
	if (!user) {
		return;
	}

	UserManager::instance().remove(user->getId());
	UserManager::instance().broadcast(S2C_ChatNotify("Server", user->getName() + " left"));
}
