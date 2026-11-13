add_executable(client
	main.cpp
	ChatClient.hpp
	ChatClient.cpp
	ChatState.hpp
	ChatState.cpp
)
target_include_directories(client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(client PRIVATE tcp::client protocol)

# 실행 파일을 다시 링크하지 않아도 설정 파일이 바뀌면 복사되도록 빌드마다 확인함
add_custom_target(client_config ALL
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${CMAKE_CURRENT_SOURCE_DIR}/client.json $<TARGET_FILE_DIR:client>/client.json
)
add_dependencies(client_config client)
