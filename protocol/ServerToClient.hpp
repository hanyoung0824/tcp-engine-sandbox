#pragma once

#include "IPacket.hpp"
#include "Log.hpp"
#include "PacketReader.hpp"
#include "PacketWriter.hpp"
#include "Protocol.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <utility>

// 서버 -> 클라이언트 공용 패킷 클래스 (응용 계층).
// 서버는 이 클래스로 만들어 보내고(write), 클라이언트는 이 클래스를 상속해 handle()을 오버라이드한 클래스로 받는다(read).
// handle()의 기본 구현은 "받는 쪽 클래스가 아니다"라는 경고만 남긴다 (보내는 쪽에서 객체를 만들 수 있도록).

// 채팅 알림. 누군가 보낸 채팅이나 서버의 알림(접속, 종료)을 모든 클라이언트에게 전한다.
class S2C_ChatNotify : public IPacket
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

	void handle(const std::shared_ptr<Session>&) override
	{
		writeLog(LogLevel::Warning, "S2C_ChatNotify has no handler on this side");
	}

public:
	std::string sender; // 보낸 사람
	std::string message; // 채팅 메시지
};
