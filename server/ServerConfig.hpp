#pragma once

#include "ServerSettings.hpp"

#include <filesystem>
#include <optional>

std::optional<tcpengine::ServerSettings> loadServerSettings(const std::filesystem::path& path);
