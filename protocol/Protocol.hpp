#pragma once

#include "PacketHeader.hpp"

#include <cstddef>
#include <cstdint>

// 채팅 프로그램의 패킷 목록 (응용 계층). 서버와 클라이언트가 함께 쓴다.
// 패킷 형식(헤더, 최대 크기)은 네트워크 계층의 PacketHeader.hpp가 정한다.
//
// 공용 패킷 클래스(IPacket 상속)는 보내는 방향별로 나눈다. 이름의 접두어로도 방향을 드러낸다.
// - ClientToServer.hpp (C2S_): 클라이언트가 보내고 서버가 받는다
// - ServerToClient.hpp (S2C_): 서버가 보내고 클라이언트가 받는다
// 공용 클래스 하나가 쓰기(write)와 읽기(read)를 모두 가져, 필드 순서가 한 곳에서만 정해진다.
// 받았을 때 할 일(handle)은 받는 쪽이 상속해 오버라이드한다 (예: server/packets/C2S_ChatHandler).
// 받는 쪽에서 다른 방향의 패킷은 등록하지 않으므로, 상대가 잘못 보내면 모르는 패킷으로 연결이 끊긴다.

enum class PacketId : std::uint16_t
{
	C2S_Chat = 1,
	S2C_ChatNotify = 2,
};

// 채팅 메시지 최대 길이 (바이트). S2C_ChatNotify(보낸 사람 + 메시지)가 maxPacketSize를 넘지 않도록 넉넉히 작게 잡는다.
constexpr std::size_t maxChatMessageLength = 1024;
