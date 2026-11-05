#pragma once

#include <cstddef>
#include <cstdint>

enum class PacketId : std::uint16_t
{
	C2S_Chat = 1,
	S2C_ChatNotify = 2,
	C2S_SetNickname = 3,
	S2C_SetNicknameResult = 4,
};

constexpr std::size_t maxChatMessageLength = 1024;

constexpr std::size_t maxNicknameLength = 16;

enum class SetNicknameResult : std::uint16_t
{
	Ok = 0,
	Empty = 1,
	Duplicate = 2,
	AlreadySet = 3,
};
