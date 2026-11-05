// 사용법: server [config]   (기본값: 실행 파일 옆의 server.json)

#include "ChatServer.hpp"
#include "ServerConfig.hpp"

#include <filesystem>
#include <optional>

int main(int argc, char* argv[])
{
	const std::filesystem::path configPath = argc > 1
		? std::filesystem::path(argv[1])
		: std::filesystem::path(argv[0]).parent_path() / "server.json";

	const std::optional<tcpengine::ServerSettings> settings = loadServerSettings(configPath);
	if (!settings) {
		return 1;
	}
	return ChatServer().run(*settings);
}
