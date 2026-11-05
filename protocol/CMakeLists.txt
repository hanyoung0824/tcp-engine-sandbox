# 프로토콜 (서버 응용과 클라이언트 응용이 함께 쓰는 약속). 바꿀 때는 서버와 클라이언트 양쪽이 함께 검토한다.
# - Protocol.hpp: 패킷 목록(PacketId)과 내용 제한
# - ClientToServer.hpp, ServerToClient.hpp: 보내는 방향별 공용 패킷 클래스 (id, 필드, read, write)
# 헤더 파일뿐이라 INTERFACE 라이브러리로 만든다.
add_library(protocol INTERFACE
	Protocol.hpp
	ClientToServer.hpp
	ServerToClient.hpp
)
target_include_directories(protocol INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
# 공용 패킷 클래스가 엔진의 패킷 틀(IPacket, PacketReader/Writer)을 쓰므로 tcp::common도 함께 연결한다.
target_link_libraries(protocol INTERFACE tcp::common)
