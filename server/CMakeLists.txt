# 채팅 서버 실행 파일 (엔진의 tcp::server를 쓴다)
# - ChatServer: Server 상속, 받을 패킷 등록과 접속/종료 처리
# - User, UserManager: 접속한 유저와 유저 목록
# - packets/: 서버가 받는 패킷 클래스 (공용 C2S_ 클래스를 상속해 handle() 오버라이드)
add_executable(server
	main.cpp
	ChatServer.hpp
	ChatServer.cpp
	User.hpp
	User.cpp
	UserManager.hpp
	UserManager.cpp
	packets/ChatPacket.hpp
	packets/ChatPacket.cpp
)
# #include "packets/ChatPacket.hpp"처럼 쓸 수 있도록 이 폴더를 include 경로에 넣는다.
target_include_directories(server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(server PRIVATE tcp::server protocol)
