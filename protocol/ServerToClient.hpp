#pragma once

#include "Protocol.hpp"
#include "tcpengine/network/packet/IPacket.hpp"
#include "tcpengine/network/packet/PacketReader.hpp"
#include "tcpengine/network/packet/PacketWriter.hpp"

#include <cstdint>
#include <string>
#include <utility>

// read()는 믿을 수 없는 입력이므로 형식과 길이 검사 필수
namespace s2c
{
class ChatNotify : public tcpengine::IPacket
{
public:
	ChatNotify() = default;
	ChatNotify(std::string sender, std::string message)
		: sender(std::move(sender)),
		message(std::move(message))
	{
	}

	std::uint16_t getPacketId() const override { return static_cast<std::uint16_t>(PacketId::S2C_ChatNotify); }

	bool read(tcpengine::PacketReader& reader) override
	{
		return reader.readString(sender) && reader.readString(message) && message.size() <= maxChatMessageLength;
	}

	void write(tcpengine::PacketWriter& writer) const override
	{
		writer.writeString(sender);
		writer.writeString(message);
	}

public:
	std::string sender;
	std::string message;
};

class SetNicknameResult : public tcpengine::IPacket
{
public:
	SetNicknameResult() = default;
	SetNicknameResult(NicknameResult result, std::string nickname)
		: result(result),
		nickname(std::move(nickname))
	{
	}

	std::uint16_t getPacketId() const override { return static_cast<std::uint16_t>(PacketId::S2C_SetNicknameResult); }

	bool read(tcpengine::PacketReader& reader) override
	{
		std::uint16_t value = 0;
		if (!reader.readUInt16(value) || value > static_cast<std::uint16_t>(NicknameResult::AlreadySet)) {
			return false;
		}
		result = static_cast<NicknameResult>(value);
		return reader.readString(nickname) && nickname.size() <= maxNicknameLength;
	}

	void write(tcpengine::PacketWriter& writer) const override
	{
		writer.writeUInt16(static_cast<std::uint16_t>(result));
		writer.writeString(nickname);
	}

public:
	NicknameResult result = NicknameResult::Ok;
	std::string nickname;
};
}
