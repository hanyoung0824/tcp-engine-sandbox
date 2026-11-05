#pragma once

#include "ClientSettings.hpp"

#include <filesystem>
#include <optional>

std::optional<tcpengine::ClientSettings> loadClientSettings(const std::filesystem::path& path);
