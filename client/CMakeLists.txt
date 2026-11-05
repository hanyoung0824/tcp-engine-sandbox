# 채팅 클라이언트 실행 파일 (엔진의 tcp::client를 쓴다)
# - ChatClient: Client 상속, 받을 패킷 등록과 입력 루프
# - packets/: 클라이언트가 받는 패킷 클래스 (공용 S2C_ 클래스를 상속해 handle() 오버라이드)
add_executable(client
	main.cpp
	ChatClient.hpp
	ChatClient.cpp
	packets/ChatNotifyPacket.hpp
	packets/ChatNotifyPacket.cpp
)
target_include_directories(client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(client PRIVATE tcp::client protocol)
