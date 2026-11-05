find_package(nlohmann_json CONFIG REQUIRED)

add_library(app_config STATIC
	JsonConfig.hpp
	JsonConfig.cpp
)
target_include_directories(app_config PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(app_config PUBLIC tcp::engine nlohmann_json::nlohmann_json)
