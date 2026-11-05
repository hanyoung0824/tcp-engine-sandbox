# 서버 (엔진 + 응용). 서버팀이 맡는 한 묶음.

# 서버 엔진: 서버 프로그램의 뼈대(Server), TCP 서버(TcpServer), 접속 받기, 세션 목록, 종료 신호 처리
add_library(server_engine STATIC
	engine/Server.hpp
	engine/Server.cpp
	engine/TcpServer.hpp
	engine/TcpServer.cpp
	engine/Listener.hpp
	engine/Listener.cpp
	engine/SessionManager.hpp
	engine/SessionManager.cpp
)
target_include_directories(server_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/engine)
target_link_libraries(server_engine PUBLIC common)

# 서버 응용 (채팅 서버 실행 파일)
# - ChatServer: Server 상속, 받을 패킷 등록과 접속/종료 처리
# - User, UserManager: 접속한 유저와 유저 목록
# - packets/: 서버가 받는 패킷 클래스 (공용 C2S_ 클래스를 상속해 handle() 오버라이드)
add_executable(server
	app/main.cpp
	app/ChatServer.hpp
	app/ChatServer.cpp
	app/User.hpp
	app/User.cpp
	app/UserManager.hpp
	app/UserManager.cpp
	app/packets/ChatPacket.hpp
	app/packets/ChatPacket.cpp
)
# #include "packets/ChatPacket.hpp"처럼 쓸 수 있도록 app 폴더를 include 경로에 넣는다.
target_include_directories(server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
target_link_libraries(server PRIVATE server_engine protocol)
