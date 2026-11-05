# 클라이언트 (엔진 + 응용). 클라이언트팀이 맡는 한 묶음.

# 클라이언트 엔진: 클라이언트 프로그램의 뼈대(Client), TCP 클라이언트(TcpClient), 접속하기, 네트워크 스레드
add_library(client_engine STATIC
	engine/Client.hpp
	engine/Client.cpp
	engine/TcpClient.hpp
	engine/TcpClient.cpp
	engine/Connector.hpp
	engine/Connector.cpp
)
target_include_directories(client_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/engine)
target_link_libraries(client_engine PUBLIC common)

# 클라이언트 응용 (채팅 클라이언트 실행 파일)
# - ChatClient: Client 상속, 받을 패킷 등록과 입력 루프
# - packets/: 클라이언트가 받는 패킷 클래스 (공용 S2C_ 클래스를 상속해 handle() 오버라이드)
add_executable(client
	app/main.cpp
	app/ChatClient.hpp
	app/ChatClient.cpp
	app/packets/ChatNotifyPacket.hpp
	app/packets/ChatNotifyPacket.cpp
)
target_include_directories(client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
target_link_libraries(client PRIVATE client_engine protocol)
