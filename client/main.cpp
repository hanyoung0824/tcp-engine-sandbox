// 사용법: client [config]   (기본값: 실행 파일 옆의 client.json)

#include "ChatClient.hpp"
#include "tcpengine/client/ClientSettings.hpp"

#include <exception>
#include <filesystem>
#include <iostream>

int main(int argc, char* argv[])
{
	const std::filesystem::path configPath = argc > 1
		? std::filesystem::path(argv[1])
		: std::filesystem::path(argv[0]).parent_path() / "client.json";

	tcpengine::ClientSettings engineSettings;

	// 응용 설정이 없으므로 엔진 설정만 읽음. 오류는 로그를 시작하기 전이라 직접 출력
	try {
		engineSettings = tcpengine::ClientSettings::create(configPath);
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << '\n';
		return 1;
	}

	ChatClient client(engineSettings);
	const int result = client.run();
	if (result == 0) {
		std::cout << "Disconnected\n";
	}
	return result;
}
