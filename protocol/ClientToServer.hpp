#pragma once

#include "IPacket.hpp"
#include "PacketReader.hpp"
#include "PacketWriter.hpp"
#include "Protocol.hpp"

#include <cstdint>
#include <string>
#include <utility>

// read()는 믿을 수 없는 입력이므로 형식과 길이 검사 필수

class C2S_Chat : public tcpengine::ClientToServerPacket
{
public:
	C2S_Chat() = default;
	explicit C2S_Chat(std::string message)
		: message(std::move(message))
	{
	}

	std::uint16_t getPacketId() const override { return static_cast<std::uint16_t>(PacketId::C2S_Chat); }

	bool read(tcpengine::PacketReader& reader) override
	{
		return reader.readString(message) && message.size() <= maxChatMessageLength;
	}

	void write(tcpengine::PacketWriter& writer) const override
	{
		writer.writeString(message);
	}

public:
	std::string message;
};

class C2S_SetNickname : public tcpengine::ClientToServerPacket
{
public:
	C2S_SetNickname() = default;
	explicit C2S_SetNickname(std::string nickname)
		: nickname(std::move(nickname))
	{
	}

	std::uint16_t getPacketId() const override { return static_cast<std::uint16_t>(PacketId::C2S_SetNickname); }

	// 너무 길면 조작된 패킷으로 보고 연결 끊음. 빈 닉네임과 중복은 결과 패킷으로 알림
	bool read(tcpengine::PacketReader& reader) override
	{
		return reader.readString(nickname) && nickname.size() <= maxNicknameLength;
	}

	void write(tcpengine::PacketWriter& writer) const override
	{
		writer.writeString(nickname);
	}

public:
	std::string nickname;
};
