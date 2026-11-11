// 사용법: server [config]   (기본값: 실행 파일 옆의 server.json)

#include "ChatServer.hpp"
#include "ChatServerSettings.hpp"
#include "tcpengine/server/ServerSettings.hpp"
#include "tcpengine/utility/ConsoleSink.hpp"
#include "tcpengine/utility/JsonConfig.hpp"

#include <exception>
#include <filesystem>
#include <iostream>
#include <memory>

int main(int argc, char* argv[])
{
	const std::filesystem::path configPath = argc > 1
		? std::filesystem::path(argv[1])
		: std::filesystem::path(argv[0]).parent_path() / "server.json";

	tcpengine::ServerSettings engineSettings;
	ChatServerSettings chatSettings;

	// 설정을 읽는 동안의 오류는 로거를 연결하기 전이라 직접 출력
	try {
		const nlohmann::json file = tcpengine::loadJsonFile(configPath);
		engineSettings = tcpengine::ServerSettings::fromJson(file);
		chatSettings = ChatServerSettings::fromJson(file);
	}
	catch (const std::exception& e) {
		std::cerr << configPath.string() << ": " << e.what() << '\n';
		return 1;
	}

	ChatServer server(engineSettings, chatSettings);
	return server.run(
		std::make_shared<tcpengine::ConsoleSink>("engine"),
		std::make_shared<tcpengine::ConsoleSink>("app"));
}
