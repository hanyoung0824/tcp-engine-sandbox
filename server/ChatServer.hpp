#pragma once

#include "ChatServerSettings.hpp"
#include "ClientToServer.hpp"
#include "UserManager.hpp"
#include "tcpengine/server/Server.hpp"

#include <memory>

// 닉네임 설정이 로그인 역할. 성공하면 세션을 시작하고 사용자를 세션에 묶음
class ChatServer : public tcpengine::Server
{
public:
	ChatServer(const tcpengine::ServerSettings& engineSettings, const ChatServerSettings& settings);

protected:
	void onStop() override;

	void onSessionResumed(const std::shared_ptr<tcpengine::Session>& session) override;

	void onSessionDetached(const std::shared_ptr<tcpengine::Session>& session) override;

	void onSessionEnded(const std::shared_ptr<tcpengine::Session>& session) override;

private:
	void handleChat(const std::shared_ptr<tcpengine::Session>& session, const c2s::Chat& packet);

	void handleSetNickname(const std::shared_ptr<tcpengine::Connection>& connection, const c2s::SetNickname& packet);

private:
	const ChatServerSettings m_settings;
	UserManager m_users;
};
