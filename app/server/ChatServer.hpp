#pragma once

#include "ServerApp.hpp"

#include <memory>

// 채팅 서버 (응용 계층).
// 한 유저가 보낸 채팅을 접속 중인 모든 유저에게 보낸다(broadcast).
// 누가 들어오거나 나가면 모든 유저에게 알린다.
// 서버를 열고 실행하는 과정은 ServerApp(네트워크 계층)이 맡고, 여기서는 이 프로그램만의 동작을 정한다.
// 접속한 유저는 UserManager로 관리한다 (엔진의 세션 목록을 직접 쓰지 않는다).
class ChatServer : public ServerApp
{
public:
	// 서버가 받을 패킷 클래스를 등록한다.
	// 새 패킷은 app/common/ClientToServer.hpp에 공용 클래스를 만들고,
	// packets/에 그 클래스를 상속해 handle()을 오버라이드한 클래스를 만들어 여기서 등록한다.
	ChatServer();

private:
	// 엔진 로그를 콘솔로 출력한다 (Info 이상).
	bool onStart() override;
	void onStop() override;

	// 접속: 유저를 만들어 목록에 넣고, 모든 유저에게 알린다 (방금 들어온 유저도 받는다).
	void onConnected(const std::shared_ptr<Session>& session) override;

	// 종료: 유저를 목록에서 빼고, 남은 유저에게 알린다.
	void onDisconnected(const std::shared_ptr<Session>& session) override;
};
