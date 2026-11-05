#pragma once

#include "Client.hpp"

#include <atomic>
#include <memory>

// 채팅 클라이언트 (응용 계층).
// 입력한 줄을 C2S_Chat으로 서버에 보내고, 서버가 보내는 S2C_ChatNotify(다른 유저의 메시지, 서버 알림)를 받는 대로 출력한다.
// 빈 줄을 입력하면 종료.
// 접속과 네트워크 스레드는 Client(클라이언트 엔진)가 맡고, 여기서는 이 프로그램만의 동작을 정한다.
class ChatClient : public Client
{
public:
	// 클라이언트가 받을 패킷 클래스를 등록한다.
	// 새 패킷은 protocol/ServerToClient.hpp에 공용 클래스를 만들고,
	// packets/에 그 클래스를 상속해 handle()을 오버라이드한 클래스를 만들어 여기서 등록한다.
	ChatClient();

private:
	// 엔진 로그는 경고 이상만 콘솔로 출력한다 (채팅 화면이 로그로 어지럽지 않도록).
	bool onStart() override;

	// 메인 스레드: 키보드 입력을 기다리다가, 입력한 줄을 C2S_Chat으로 보낸다.
	void onRun() override;

	// 서버가 먼저 끊었으면, 메인 스레드가 입력을 기다리고 있으므로 Enter를 누르라고 안내한다.
	void onDisconnected(const std::shared_ptr<Session>& session) override;

private:
	std::atomic<bool> m_quitting{ false }; // 내가 빈 줄로 끝냈는지 (네트워크 스레드도 읽는다)
};
