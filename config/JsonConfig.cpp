#include "JsonConfig.hpp"

#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

std::optional<nlohmann::json> loadJsonFile(const std::filesystem::path& path)
{
	std::ifstream file(path);
	if (!file) {
		std::cerr << "Cannot open " << path.string() << '\n';
		return std::nullopt;
	}

	try {
		nlohmann::json json = nlohmann::json::parse(file);
		if (!json.is_object()) {
			std::cerr << path.string() << ": top level must be an object\n";
			return std::nullopt;
		}
		return json;
	}
	catch (const nlohmann::json::exception& e) {
		std::cerr << path.string() << ": " << e.what() << '\n';
		return std::nullopt;
	}
}

unsigned short readPort(const nlohmann::json& json)
{
	const int value = json.at("port").get<int>();
	if (value < 1 || value > 65535) {
		throw std::invalid_argument("port must be between 1 and 65535");
	}
	return static_cast<unsigned short>(value);
}

tcpengine::HeartbeatSettings readHeartbeat(const nlohmann::json& json)
{
	const nlohmann::json& heartbeat = json.at("heartbeat");
	const auto interval = heartbeat.at("interval").get<std::int64_t>();
	const auto timeout = heartbeat.at("timeout").get<std::int64_t>();
	if (interval < 0 || timeout < 0) {
		throw std::invalid_argument("heartbeat values must not be negative");
	}

	tcpengine::HeartbeatSettings settings;
	settings.interval = std::chrono::seconds(interval);
	settings.timeout = std::chrono::seconds(timeout);
	return settings;
}

tcpengine::LogLevel readLogLevel(const nlohmann::json& json)
{
	const std::string name = json.at("logLevel").get<std::string>();
	if (name == "debug") {
		return tcpengine::LogLevel::Debug;
	}
	if (name == "info") {
		return tcpengine::LogLevel::Info;
	}
	if (name == "warning") {
		return tcpengine::LogLevel::Warning;
	}
	if (name == "error") {
		return tcpengine::LogLevel::Error;
	}
	throw std::invalid_argument("logLevel must be one of debug, info, warning, error");
}
