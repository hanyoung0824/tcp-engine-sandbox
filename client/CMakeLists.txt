add_executable(client
	main.cpp
	ChatClient.hpp
	ChatClient.cpp
	ChatState.hpp
	ChatState.cpp
	ServerToClientHandler.hpp
	ServerToClientHandler.cpp
)
target_include_directories(client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(client PRIVATE tcp::client protocol)
