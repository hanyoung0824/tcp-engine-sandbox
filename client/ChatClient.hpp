#pragma once

#include "ChatState.hpp"
#include "Client.hpp"
#include "ServerToClient.hpp"

#include <atomic>
#include <memory>

// onRun()은 메인 스레드, handle*()과 onDisconnected()는 네트워크 스레드에서 실행
class ChatClient : public tcpengine::Client
{
public:
	ChatClient();

private:
	void onRun() override;

	void onDisconnected(const std::shared_ptr<tcpengine::Session>& session) override;

	void handleChatNotify(const S2C_ChatNotify& packet);

	void handleSetNicknameResult(const S2C_SetNicknameResult& packet);

private:
	ChatState m_state;
	std::atomic<bool> m_quitting{ false }; // 메인 스레드가 쓰고 네트워크 스레드가 읽으므로 atomic
};
