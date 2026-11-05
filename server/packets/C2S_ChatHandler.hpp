#pragma once

#include "ClientToServer.hpp"

// C2S_Chat을 받았을 때 (서버): 보낸 유저의 이름을 붙여 S2C_ChatNotify로 모든 유저에게 보낸다.
// 필드와 읽기/쓰기는 공용 클래스 C2S_Chat에 있고, 여기서는 handle()만 오버라이드한다.
class C2S_ChatHandler : public C2S_Chat
{
public:
	void handle(const std::shared_ptr<Session>& session) override;
};
