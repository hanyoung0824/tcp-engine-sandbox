// TCP 채팅 서버
// Ctrl+C(SIGINT) 또는 SIGTERM을 받으면 모든 연결을 닫고 종료한다.
//
// 역할 분리:
// - 네트워크 계층 (net/, 범용): ServerApp이 서버를 열고 세션 이벤트를 가상 함수로 연결해 실행한다.
// - 응용 계층 (app/, 이 채팅 프로그램)
//   - app/server/ChatServer: ServerApp을 상속해 이 프로그램의 동작(접속/종료 알림, 로그 정책)을 정한다.
//   - app/server/User, UserManager: 접속한 유저와 유저 목록
//   - app/server/packets/: 서버가 받는 패킷 클래스 (ChatServer 생성자에서 등록)
//   - app/common/: 서버와 클라이언트가 함께 쓰는 패킷 목록과 공용 패킷 클래스

#include "ChatServer.hpp"

int main()
{
	constexpr unsigned short port = 12345;
	return ChatServer().run(port);
}
