add_executable(server
	main.cpp
	ChatServer.hpp
	ChatServer.cpp
	User.hpp
	User.cpp
	UserManager.hpp
	UserManager.cpp
)
target_include_directories(server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(server PRIVATE tcp::server protocol)

add_custom_command(TARGET server POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${CMAKE_CURRENT_SOURCE_DIR}/server.json $<TARGET_FILE_DIR:server>/server.json
)
