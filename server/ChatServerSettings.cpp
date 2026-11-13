#include "ChatServerSettings.hpp"

#include <nlohmann/json.hpp>

#include <exception>
#include <stdexcept>
#include <string>

void ChatServerSettings::validate() const
{
	if (noticeSender.empty()) {
		throw std::invalid_argument("noticeSender must not be empty");
	}
	if (chatLimit.messages <= 0 || chatLimit.perSeconds <= 0) {
		throw std::invalid_argument("chatLimit messages and perSeconds must be positive");
	}
}

ChatServerSettings ChatServerSettings::fromJson(const nlohmann::json& file)
{
	try {
		const nlohmann::json& app = file.at("app");
		ChatServerSettings settings;
		settings.noticeSender = app.at("noticeSender").get<std::string>();
		settings.chatLimit.messages = app.at("chatLimit").at("messages").get<double>();
		settings.chatLimit.perSeconds = app.at("chatLimit").at("perSeconds").get<double>();
		settings.validate();
		return settings;
	}
	catch (const std::exception& e) {
		throw std::runtime_error(std::string("invalid app settings: ") + e.what());
	}
}
