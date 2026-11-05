add_executable(client
	main.cpp
	ChatClient.hpp
	ChatClient.cpp
	ClientConfig.hpp
	ClientConfig.cpp
	ChatState.hpp
	ChatState.cpp
)
target_include_directories(client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(client PRIVATE tcp::engine protocol app_config)

add_custom_command(TARGET client POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${CMAKE_CURRENT_SOURCE_DIR}/client.json $<TARGET_FILE_DIR:client>/client.json
)
