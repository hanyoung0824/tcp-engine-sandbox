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
	ChatServerSettings settings;
	const auto app = file.find("app");
	if (app == file.end()) {
		return settings;
	}

	try {
		settings.noticeSender = app->value("noticeSender", settings.noticeSender);
		if (const auto chatLimit = app->find("chatLimit"); chatLimit != app->end()) {
			settings.chatLimit.messages = chatLimit->at("messages").get<double>();
			settings.chatLimit.perSeconds = chatLimit->at("perSeconds").get<double>();
		}
		settings.validate();
	}
	catch (const std::exception& e) {
		throw std::runtime_error(std::string("invalid app settings: ") + e.what());
	}
	return settings;
}
