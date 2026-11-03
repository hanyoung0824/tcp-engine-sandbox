# 네트워크 계층 (범용). 접속, 세션, 바이트 송수신과 기본 패킷 틀을 제공하므로 다른 프로젝트에도 그대로 쓸 수 있다.
# 어떤 패킷이 있는지(IPacket을 상속한 패킷 클래스)와 받았을 때 할 일(handle)은 쓰는 쪽(응용 계층)이 정한다.
# 쓰는 쪽은 Server 또는 Client에 수신 콜백(받은 바이트를 해석하는 함수, 보통 PacketManager)을 넘겨 시작한다.
# 각 라이브러리는 자기 폴더를 include 경로로 공개하므로, 링크하는 쪽에서 #include "Session.hpp"처럼 쓸 수 있다.

# 서버와 클라이언트가 함께 쓰는 것
# - Session: 연결 하나 (바이트만 다룬다)
# - 패킷 틀: 형식(PacketHeader), 만들기(PacketWriter), 읽기(PacketReader), 받은 바이트를 패킷 객체로 바꿔 처리하기(IPacket, PacketManager),
#   보내기 도우미(PacketHelper)
# - 로그 창구(Log): 엔진은 직접 출력하지 않고 응용이 등록한 함수로 로그를 넘긴다
add_library(net_common STATIC
	common/Log.hpp
	common/Log.cpp
	common/Session.hpp
	common/Session.cpp
	common/PacketHeader.hpp
	common/IPacket.hpp
	common/PacketHelper.hpp
	common/PacketWriter.hpp
	common/PacketWriter.cpp
	common/PacketReader.hpp
	common/PacketReader.cpp
	common/PacketManager.hpp
	common/PacketManager.cpp
)
target_include_directories(net_common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/common)
# 헤더가 Asio를 쓰므로 Asio와 스레드 라이브러리도 함께 공개한다.
target_link_libraries(net_common PUBLIC Boost::asio Threads::Threads)

# 서버용: 서버 프로그램의 뼈대(ServerApp), 접속 받기, 세션 목록, 종료 신호 처리
add_library(net_server STATIC
	server/ServerApp.hpp
	server/ServerApp.cpp
	server/Server.hpp
	server/Server.cpp
	server/Listener.hpp
	server/Listener.cpp
	server/SessionManager.hpp
	server/SessionManager.cpp
)
target_include_directories(net_server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/server)
target_link_libraries(net_server PUBLIC net_common)

# 클라이언트용: 클라이언트 프로그램의 뼈대(ClientApp), 접속하기, 네트워크 스레드
add_library(net_client STATIC
	client/ClientApp.hpp
	client/ClientApp.cpp
	client/Client.hpp
	client/Client.cpp
	client/Connector.hpp
	client/Connector.cpp
)
target_include_directories(net_client PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/client)
target_link_libraries(net_client PUBLIC net_common)
