// 사용법: client [config]   (기본값: 실행 파일 옆의 client.json)

#include "ChatClient.hpp"

#include <filesystem>
#include <iostream>

int main(int argc, char* argv[])
{
	const std::filesystem::path configPath = argc > 1
		? std::filesystem::path(argv[1])
		: std::filesystem::path(argv[0]).parent_path() / "client.json";

	const int result = ChatClient().run(configPath);
	if (result == 0) {
		std::cout << "Disconnected\n";
	}
	return result;
}
