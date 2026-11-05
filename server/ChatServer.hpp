#pragma once

#include "ClientToServer.hpp"
#include "Server.hpp"
#include "UserManager.hpp"

#include <memory>

class ChatServer : public tcpengine::Server
{
public:
	ChatServer();

private:
	void onStop() override;

	void onConnected(const std::shared_ptr<tcpengine::Session>& session) override;

	void onDisconnected(const std::shared_ptr<tcpengine::Session>& session) override;

	void handleChat(const std::shared_ptr<tcpengine::Session>& session, const C2S_Chat& packet);

	void handleSetNickname(const std::shared_ptr<tcpengine::Session>& session, const C2S_SetNickname& packet);

private:
	UserManager m_users;
};
