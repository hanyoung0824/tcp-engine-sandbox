#pragma once

#include "ClientToServer.hpp"
#include "UserManager.hpp"
#include "tcpengine/server/Server.hpp"

#include <memory>

class ChatServer : public tcpengine::Server
{
public:
	ChatServer();

protected:
	bool onStart() override;

	void onStop() override;

	void onSessionConnected(const std::shared_ptr<tcpengine::Session>& session) override;

	void onSessionDisconnected(const std::shared_ptr<tcpengine::Session>& session) override;

private:
	void handleChat(const std::shared_ptr<tcpengine::Session>& session, const c2s::Chat& packet);

	void handleSetNickname(const std::shared_ptr<tcpengine::Session>& session, const c2s::SetNickname& packet);

private:
	UserManager m_users;
};
