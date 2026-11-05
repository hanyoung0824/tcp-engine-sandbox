add_executable(server
	main.cpp
	ChatServer.hpp
	ChatServer.cpp
	User.hpp
	User.cpp
	UserManager.hpp
	UserManager.cpp
	ClientToServerHandler.hpp
	ClientToServerHandler.cpp
)
target_include_directories(server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(server PRIVATE tcp::server protocol)
