# 응용(패킷) 계층. 이 채팅 프로그램의 패킷 목록과 패킷 처리, 실행 파일.

# 서버와 클라이언트가 함께 쓰는 것. 헤더 파일뿐이라 INTERFACE 라이브러리로 만든다.
# - Protocol.hpp: 패킷 목록(PacketId)과 내용 제한
# - ClientToServer.hpp, ServerToClient.hpp: 보내는 방향별 공용 패킷 클래스 (id, 필드, read, write)
# - ConsoleLog.hpp: 엔진 로그를 콘솔로 출력하는 로그 정책
add_library(app_common INTERFACE
	common/Protocol.hpp
	common/ClientToServer.hpp
	common/ServerToClient.hpp
	common/ConsoleLog.hpp
)
target_include_directories(app_common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/common)
# net의 패킷 틀과 로그 창구를 쓰므로 net_common도 함께 연결한다.
target_link_libraries(app_common INTERFACE net_common)

# 서버 실행 파일
# - ChatServer: ServerApp 상속, 받을 패킷 등록과 접속/종료 처리
# - User, UserManager: 접속한 유저와 유저 목록
# - packets/: 서버가 받는 패킷 클래스 (공용 C2S_ 클래스를 상속해 handle() 오버라이드)
add_executable(server
	server/main.cpp
	server/ChatServer.hpp
	server/ChatServer.cpp
	server/User.hpp
	server/User.cpp
	server/UserManager.hpp
	server/UserManager.cpp
	server/packets/ChatPacket.hpp
	server/packets/ChatPacket.cpp
)
# #include "packets/ChatPacket.hpp"처럼 쓸 수 있도록 server 폴더를 include 경로에 넣는다.
target_include_directories(server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/server)
target_link_libraries(server PRIVATE app_common net_server)

# 클라이언트 실행 파일
# - ChatClient: ClientApp 상속, 받을 패킷 등록과 입력 루프
# - packets/: 클라이언트가 받는 패킷 클래스 (공용 S2C_ 클래스를 상속해 handle() 오버라이드)
add_executable(client
	client/main.cpp
	client/ChatClient.hpp
	client/ChatClient.cpp
	client/packets/ChatNotifyPacket.hpp
	client/packets/ChatNotifyPacket.cpp
)
target_include_directories(client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/client)
target_link_libraries(client PRIVATE app_common net_client)
