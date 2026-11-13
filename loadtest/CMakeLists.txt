find_package(boost_asio CONFIG REQUIRED)
find_package(Threads REQUIRED)

add_executable(loadtest
	main.cpp
	LoadTest.hpp
	LoadTest.cpp
	Bot.hpp
	Bot.cpp
)
target_include_directories(loadtest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(loadtest PRIVATE protocol Boost::asio Threads::Threads)
if(WIN32)
	target_compile_definitions(loadtest PRIVATE _WIN32_WINNT=0x0A00)
endif()

# 부하 테스트용 서버 설정. server 실행 파일에 인자로 넘김
# 실행 파일을 다시 링크하지 않아도 설정 파일이 바뀌면 복사되도록 빌드마다 확인함
add_custom_target(loadtest_config ALL
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${CMAKE_CURRENT_SOURCE_DIR}/loadtest-server.json $<TARGET_FILE_DIR:loadtest>/loadtest-server.json
)
add_dependencies(loadtest_config loadtest)
