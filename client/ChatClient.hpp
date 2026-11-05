#pragma once

#include "Client.hpp"

#include <atomic>
#include <memory>

// onRun()은 메인 스레드, handle()과 onDisconnected()는 네트워크 스레드에서 돈다.
class ChatClient : public Client
{
public:
	ChatClient();

private:
	bool onStart() override;

	void onRun() override;

	void onDisconnected(const std::shared_ptr<Session>& session) override;

private:
	std::atomic<bool> m_quitting{ false }; // 메인 스레드가 쓰고 네트워크 스레드가 읽으므로 atomic이다.
};
