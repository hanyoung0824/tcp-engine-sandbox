#include "ServerConfig.hpp"

#include "JsonConfig.hpp"

#include <exception>
#include <iostream>

std::optional<tcpengine::ServerSettings> loadServerSettings(const std::filesystem::path& path)
{
	const std::optional<nlohmann::json> json = loadJsonFile(path);
	if (!json) {
		return std::nullopt;
	}

	try {
		tcpengine::ServerSettings settings;
		settings.port = readPort(*json);
		settings.heartbeat = readHeartbeat(*json);
		settings.logLevel = readLogLevel(*json);
		return settings;
	}
	catch (const std::exception& e) {
		std::cerr << path.string() << ": " << e.what() << '\n';
		return std::nullopt;
	}
}
