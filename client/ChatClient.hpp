#pragma once

#include "ChatState.hpp"
#include "ServerToClient.hpp"
#include "tcpengine/client/Client.hpp"

#include <atomic>
#include <memory>

// onRun()은 메인 스레드, handle*()과 onDisconnected()는 네트워크 스레드에서 실행
class ChatClient : public tcpengine::Client
{
public:
	explicit ChatClient(const tcpengine::ClientSettings& settings);

protected:
	void onRun() override;

	void onDisconnected(const std::shared_ptr<tcpengine::Connection>& connection) override;

private:
	void handleChatNotify(const s2c::ChatNotify& packet);

	void handleSetNicknameResult(const s2c::SetNicknameResult& packet);

private:
	ChatState m_state;
	std::atomic<bool> m_quitting{ false }; // 메인 스레드가 쓰고 네트워크 스레드가 읽으므로 atomic
};
