#pragma once

#include "tcpengine/network/packet/Packet.hpp"

#include <cstddef>
#include <cstdint>

enum class PacketId : std::uint16_t
{
	C2S_Chat = tcpengine::firstAppPacketId,
	S2C_ChatNotify,
	C2S_SetNickname,
	S2C_SetNicknameResult,
};

// 패킷 형식이나 의미를 바꾸면 올림. 서버와 클라이언트가 다르면 서버가 접속 직후 끊음
constexpr std::uint16_t protocolVersion = 1;

constexpr std::size_t maxChatMessageLength = 1024;

constexpr std::size_t maxNicknameLength = 16;

enum class NicknameResult : std::uint16_t
{
	Ok = 0,
	Empty = 1,
	Duplicate = 2,
	AlreadySet = 3,
};
