#pragma once

#include "ServerToClient.hpp"

// S2C_ChatNotify를 받았을 때 (클라이언트): "[보낸 사람] 메시지"로 출력한다.
// 필드와 읽기/쓰기는 공용 클래스 S2C_ChatNotify에 있고, 여기서는 handle()만 오버라이드한다.
class ChatNotifyPacket : public S2C_ChatNotify
{
public:
	void handle(const std::shared_ptr<Session>& session) override;
};
