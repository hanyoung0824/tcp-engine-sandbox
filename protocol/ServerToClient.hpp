#pragma once

#include "IPacket.hpp"
#include "PacketReader.hpp"
#include "PacketWriter.hpp"
#include "Protocol.hpp"

#include <cstdint>
#include <string>
#include <utility>

// read()는 믿을 수 없는 입력을 읽으므로 형식과 길이를 검사한다.

class S2C_ChatNotify : public ServerToClientPacket
{
public:
	S2C_ChatNotify() = default;
	S2C_ChatNotify(std::string sender, std::string message)
		: sender(std::move(sender)),
		message(std::move(message))
	{
	}

	std::uint16_t getPacketId() const override { return static_cast<std::uint16_t>(PacketId::S2C_ChatNotify); }

	bool read(PacketReader& reader) override
	{
		return reader.readString(sender) && reader.readString(message) && message.size() <= maxChatMessageLength;
	}

	void write(PacketWriter& writer) const override
	{
		writer.writeString(sender);
		writer.writeString(message);
	}

public:
	std::string sender;
	std::string message;
};

class S2C_SetNicknameResult : public ServerToClientPacket
{
public:
	S2C_SetNicknameResult() = default;
	S2C_SetNicknameResult(SetNicknameResult result, std::string nickname)
		: result(result),
		nickname(std::move(nickname))
	{
	}

	std::uint16_t getPacketId() const override { return static_cast<std::uint16_t>(PacketId::S2C_SetNicknameResult); }

	bool read(PacketReader& reader) override
	{
		std::uint16_t value = 0;
		if (!reader.readUInt16(value) || value > static_cast<std::uint16_t>(SetNicknameResult::AlreadySet)) {
			return false;
		}
		result = static_cast<SetNicknameResult>(value);
		return reader.readString(nickname) && nickname.size() <= maxNicknameLength;
	}

	void write(PacketWriter& writer) const override
	{
		writer.writeUInt16(static_cast<std::uint16_t>(result));
		writer.writeString(nickname);
	}

public:
	SetNicknameResult result = SetNicknameResult::Ok;
	std::string nickname;
};
