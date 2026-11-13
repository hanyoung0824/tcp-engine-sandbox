add_executable(server
	main.cpp
	ChatServer.hpp
	ChatServerSettings.hpp
	ChatServerSettings.cpp
	ChatServer.cpp
	User.hpp
	User.cpp
	UserManager.hpp
	UserManager.cpp
)
target_include_directories(server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(server PRIVATE tcp::server protocol)

# 실행 파일을 다시 링크하지 않아도 설정 파일이 바뀌면 복사되도록 빌드마다 확인함
add_custom_target(server_config ALL
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${CMAKE_CURRENT_SOURCE_DIR}/server.json $<TARGET_FILE_DIR:server>/server.json
)
add_dependencies(server_config server)
