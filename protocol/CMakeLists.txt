add_library(protocol INTERFACE
	Protocol.hpp
	ClientToServer.hpp
	ServerToClient.hpp
)
target_include_directories(protocol INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(protocol INTERFACE tcp::common)
