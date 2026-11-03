// TCP 채팅 클라이언트
// 사용법: client [host] [port]   (기본값 127.0.0.1 12345)
//
// 역할 분리:
// - 네트워크 계층 (net/, 범용): ClientApp이 접속하고 네트워크 스레드를 돌리며 세션 이벤트를 가상 함수로 연결한다.
// - 응용 계층 (app/, 이 채팅 프로그램)
//   - app/client/ChatClient: ClientApp을 상속해 이 프로그램의 동작(입력 루프, 종료 안내, 로그 정책)을 정한다.
//   - app/client/packets/: 클라이언트가 받는 패킷 클래스 (ChatClient 생성자에서 등록)
//   - app/common/: 서버와 클라이언트가 함께 쓰는 패킷 목록과 공용 패킷 클래스

#include "ChatClient.hpp"

#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
	const std::string host = argc > 1 ? argv[1] : "127.0.0.1";
	const unsigned short port = argc > 2 ? static_cast<unsigned short>(std::stoi(argv[2])) : 12345;

	// 접속에 실패하면 이유는 엔진 로그(경고)로 출력된다.
	const int result = ChatClient().run(host, port);
	if (result == 0) {
		std::cout << "Disconnected\n";
	}
	return result;
}
