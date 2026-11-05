// TCP 채팅 서버
// Ctrl+C(SIGINT) 또는 SIGTERM을 받으면 모든 연결을 닫고 종료한다.
//
// 역할 분리:
// - common/: 네트워크 공용 (Session, 패킷 틀, 로그). 클라이언트 엔진과 함께 쓴다.
// - protocol/: 서버와 클라이언트가 함께 쓰는 패킷 목록과 공용 패킷 클래스
// - server/engine/: 서버 엔진. Server(프로그램의 뼈대)가 서버를 열고 세션 이벤트를 가상 함수로 연결해 실행한다.
// - server/app/: 서버 응용 (이 채팅 서버)
//   - ChatServer: Server를 상속해 이 프로그램의 동작(접속/종료 알림, 로그 정책)을 정한다.
//   - User, UserManager: 접속한 유저와 유저 목록
//   - packets/: 서버가 받는 패킷 클래스 (ChatServer 생성자에서 등록)

#include "ChatServer.hpp"

int main()
{
	constexpr unsigned short port = 12345;
	return ChatServer().run(port);
}
