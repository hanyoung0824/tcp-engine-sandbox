cmake_minimum_required(VERSION 3.25)
project(asio_multithread_test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
	# Sources are UTF-8 (Korean comments).
	add_compile_options(/utf-8)
endif()

# TCP 네트워크 엔진 (별도 저장소 hanyoung0824/net-tcp-engine을 git submodule로 가져온다).
# tcp::common, tcp::server, tcp::client 타깃을 공개한다. Boost.Asio는 이 저장소의 vcpkg.json으로 설치된다.
set(ENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/net-tcp-engine)
if(NOT EXISTS ${ENGINE_DIR}/CMakeLists.txt)
	message(FATAL_ERROR "Engine submodule is missing. Run: git submodule update --init")
endif()
add_subdirectory(${ENGINE_DIR} ${CMAKE_BINARY_DIR}/engine)

# 서버나 클라이언트 한쪽만 빌드할 수 있다 (예: cmake --preset ... -DBUILD_CLIENT=OFF).
option(BUILD_SERVER "Build the chat server" ON)
option(BUILD_CLIENT "Build the chat client" ON)

# protocol/: 프로토콜 (서버·클라이언트가 함께 쓰는 패킷 정의)
# server/: 채팅 서버 (tcp::server 사용)
# client/: 채팅 클라이언트 (tcp::client 사용)
add_subdirectory(protocol)

if(BUILD_SERVER)
	add_subdirectory(server)
endif()

if(BUILD_CLIENT)
	add_subdirectory(client)
endif()
