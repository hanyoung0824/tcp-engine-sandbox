#include "ClientConfig.hpp"

#include "JsonConfig.hpp"

#include <exception>
#include <iostream>
#include <string>

std::optional<tcpengine::ClientSettings> loadClientSettings(const std::filesystem::path& path)
{
	const std::optional<nlohmann::json> json = loadJsonFile(path);
	if (!json) {
		return std::nullopt;
	}

	try {
		tcpengine::ClientSettings settings;
		settings.host = json->at("host").get<std::string>();
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
