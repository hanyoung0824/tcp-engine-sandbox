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
		settings.validate();
	}
	catch (const std::exception& e) {
		throw std::runtime_error(std::string("invalid app settings: ") + e.what());
	}
	return settings;
}
