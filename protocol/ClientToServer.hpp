#pragma once

#include "Protocol.hpp"
#include "tcpengine/network/packet/Packet.hpp"
#include "tcpengine/network/packet/PacketReader.hpp"
#include "tcpengine/network/packet/PacketWriter.hpp"

#include <string>
#include <utility>

// 받은 값은 믿을 수 없으므로 validate()에서 값 규칙 검사 필수
namespace c2s
{
class Chat : public tcpengine::Packet<PacketId::C2S_Chat>
{
public:
	Chat() = default;
	explicit Chat(std::string message)
		: message(std::move(message))
	{
	}

	bool read(tcpengine::PacketReader& reader) override
	{
		return reader.readString(message);
	}

	bool validate() const override
	{
		return message.size() <= maxChatMessageLength;
	}

	void write(tcpengine::PacketWriter& writer) const override
	{
		writer.writeString(message);
	}

public:
	std::string message;
};

class SetNickname : public tcpengine::Packet<PacketId::C2S_SetNickname>
{
public:
	SetNickname() = default;
	explicit SetNickname(std::string nickname)
		: nickname(std::move(nickname))
	{
	}

	bool read(tcpengine::PacketReader& reader) override
	{
		return reader.readString(nickname);
	}

	// 너무 길면 조작된 패킷으로 보고 연결 끊음. 빈 닉네임과 중복은 결과 패킷으로 알림
	bool validate() const override
	{
		return nickname.size() <= maxNicknameLength;
	}

	void write(tcpengine::PacketWriter& writer) const override
	{
		writer.writeString(nickname);
	}

public:
	std::string nickname;
};
}
