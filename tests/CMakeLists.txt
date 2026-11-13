# 서버를 띄워 시나리오를 확인하는 테스트. Python이 없으면 등록하지 않음
find_package(Python3 COMPONENTS Interpreter)
if(NOT Python3_Interpreter_FOUND OR NOT TARGET server)
	message(STATUS "Server tests disabled (needs Python 3 and the server target)")
	return()
endif()

set(SERVER_TEST_SCENARIOS
	chat chat_limit receive_limit heartbeat send_queue_limit
	shutdown force_shutdown login_timeout max_connections version_mismatch logout
)
set(SERVER_TEST_ARGUMENTS
	--server $<TARGET_FILE:server>
	--base-config ${CMAKE_SOURCE_DIR}/server/server.json
)
if(TARGET loadtest)
	list(APPEND SERVER_TEST_SCENARIOS load)
	list(APPEND SERVER_TEST_ARGUMENTS --loadtest $<TARGET_FILE:loadtest>)
endif()
if(TARGET client)
	list(APPEND SERVER_TEST_SCENARIOS reconnect)
	list(APPEND SERVER_TEST_ARGUMENTS --client $<TARGET_FILE:client> --client-config ${CMAKE_SOURCE_DIR}/client/client.json)
endif()

foreach(scenario IN LISTS SERVER_TEST_SCENARIOS)
	add_test(NAME server.${scenario}
		COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/server_tests.py ${SERVER_TEST_ARGUMENTS} ${scenario}
	)
	set_tests_properties(server.${scenario} PROPERTIES TIMEOUT 60)
endforeach()
