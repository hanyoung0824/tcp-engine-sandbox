cmake_minimum_required(VERSION 3.25)
project(asio_multithread_test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Boost.Asio 1.88, installed by vcpkg from vcpkg.json.
find_package(boost_asio CONFIG REQUIRED)
find_package(Threads REQUIRED)

if(WIN32)
	# Target Windows 10+ (silences Asio's "Please define _WIN32_WINNT" warning).
	add_compile_definitions(_WIN32_WINNT=0x0A00)
endif()

if(MSVC)
	# Sources are UTF-8 (Korean comments).
	add_compile_options(/utf-8)
endif()

# 서버나 클라이언트 한쪽만 빌드할 수 있다 (예: cmake --preset ... -DBUILD_CLIENT=OFF).
option(BUILD_SERVER "Build the server (engine + app)" ON)
option(BUILD_CLIENT "Build the client (engine + app)" ON)

# common/: 네트워크 공용 (서버·클라이언트 엔진이 함께 쓴다)
# protocol/: 프로토콜 (서버·클라이언트 응용이 함께 쓰는 패킷 정의)
# server/: 서버 엔진 + 서버 응용
# client/: 클라이언트 엔진 + 클라이언트 응용
add_subdirectory(common)
add_subdirectory(protocol)

if(BUILD_SERVER)
	add_subdirectory(server)
endif()

if(BUILD_CLIENT)
	add_subdirectory(client)
endif()
