#pragma once

#include "tcpengine/network/packet/IPacket.hpp"

#include <cstddef>
#include <cstdint>

enum class PacketId : std::uint16_t
{
	C2S_Chat = tcpengine::firstAppPacketId,
	S2C_ChatNotify,
	C2S_SetNickname,
	S2C_SetNicknameResult,
};

constexpr std::size_t maxChatMessageLength = 1024;

constexpr std::size_t maxNicknameLength = 16;

enum class NicknameResult : std::uint16_t
{
	Ok = 0,
	Empty = 1,
	Duplicate = 2,
	AlreadySet = 3,
};
