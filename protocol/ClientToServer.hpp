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

// 클라이언트 -> 서버 공용 패킷 클래스 (응용 계층).
// 클라이언트는 이 클래스로 만들어 보내고(write), 서버는 이 클래스를 상속해 handle()을 오버라이드한 클래스로 받는다(read).
// handle()의 기본 구현은 "받는 쪽 클래스가 아니다"라는 경고만 남긴다 (보내는 쪽에서 객체를 만들 수 있도록).

// 채팅 보내기
class C2S_Chat : public IPacket
{
public:
	C2S_Chat() = default;
	explicit C2S_Chat(std::string message)
		: message(std::move(message))
	{
	}

	std::uint16_t getPacketId() const override { return static_cast<std::uint16_t>(PacketId::C2S_Chat); }

	// 받은 데이터는 믿을 수 없으므로 형식과 길이를 모두 검사한다.
	bool read(PacketReader& reader) override
	{
		return reader.readString(message) && message.size() <= maxChatMessageLength;
	}

	void write(PacketWriter& writer) const override
	{
		writer.writeString(message);
	}

	void handle(const std::shared_ptr<Session>&) override
	{
		writeLog(LogLevel::Warning, "C2S_Chat has no handler on this side");
	}

public:
	std::string message; // 채팅 메시지
};
