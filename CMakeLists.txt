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

# net/: 네트워크 계층 (범용, 패킷 형식은 모른다)
# app/: 응용(패킷) 계층 (이 채팅 프로그램의 패킷과 실행 파일)
add_subdirectory(net)
add_subdirectory(app)
