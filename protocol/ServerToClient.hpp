#pragma once

#include "Protocol.hpp"
#include "tcpengine/network/packet/Packet.hpp"
#include "tcpengine/network/packet/PacketReader.hpp"
#include "tcpengine/network/packet/PacketWriter.hpp"

#include <cstdint>
#include <string>
#include <utility>

// 받은 값은 믿을 수 없으므로 validate()에서 값 규칙 검사 필수
namespace s2c
{
class ChatNotify : public tcpengine::Packet<PacketId::S2C_ChatNotify>
{
public:
	ChatNotify() = default;
	ChatNotify(std::string sender, std::string message)
		: sender(std::move(sender)),
		message(std::move(message))
	{
	}

	bool read(tcpengine::PacketReader& reader) override
	{
		return reader.readString(sender) && reader.readString(message);
	}

	bool validate() const override
	{
		return message.size() <= maxChatMessageLength;
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

class SetNicknameResult : public tcpengine::Packet<PacketId::S2C_SetNicknameResult>
{
public:
	SetNicknameResult() = default;
	SetNicknameResult(NicknameResult result, std::string nickname)
		: result(result),
		nickname(std::move(nickname))
	{
	}

	bool read(tcpengine::PacketReader& reader) override
	{
		std::uint16_t value = 0;
		if (!reader.readUInt16(value)) {
			return false;
		}
		result = static_cast<NicknameResult>(value);
		return reader.readString(nickname);
	}

	bool validate() const override
	{
		return result <= NicknameResult::AlreadySet && nickname.size() <= maxNicknameLength;
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
