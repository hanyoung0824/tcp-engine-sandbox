// TCP 채팅 서버
// Ctrl+C(SIGINT) 또는 SIGTERM을 받으면 모든 연결을 닫고 종료한다.
//
// 역할 분리:
// - external/net-tcp-engine/: TCP 네트워크 엔진 (별도 저장소, git submodule)
//   Server(프로그램의 뼈대)가 서버를 열고 세션 이벤트를 가상 함수로 연결해 실행한다.
// - protocol/: 서버와 클라이언트가 함께 쓰는 패킷 목록과 공용 패킷 클래스
// - server/: 이 채팅 서버
//   - ChatServer: Server를 상속해 이 프로그램의 동작(접속/종료 알림, 로그 정책)을 정한다.
//   - User, UserManager: 접속한 유저와 유저 목록
//   - packets/: 서버가 받는 패킷 클래스 (ChatServer 생성자에서 등록)

#include "ChatServer.hpp"

int main()
{
	constexpr unsigned short port = 12345;
	return ChatServer().run(port);
}
