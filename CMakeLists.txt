cmake_minimum_required(VERSION 3.25)
project(tcp_engine_sandbox LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
	add_compile_options(/utf-8)
endif()

# TCP 네트워크 엔진 (git submodule)
set(ENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/tcp-engine)
if(NOT EXISTS ${ENGINE_DIR}/CMakeLists.txt)
	message(FATAL_ERROR "Engine submodule is missing. Run: git submodule update --init")
endif()
add_subdirectory(${ENGINE_DIR} ${CMAKE_BINARY_DIR}/engine)

option(BUILD_SERVER "Build the chat server" ON)
option(BUILD_CLIENT "Build the chat client" ON)

add_subdirectory(protocol)

if(BUILD_SERVER)
	add_subdirectory(server)
endif()

if(BUILD_CLIENT)
	add_subdirectory(client)
endif()
