#pragma once

#include "ServerToClient.hpp"

// 핸들러 이름은 <패킷 이름>Handler로 짓고 handle()을 오버라이드한 뒤 ChatClient 생성자에서 registerPacket<T>()로 등록한다.
class S2C_ChatNotifyHandler : public S2C_ChatNotify
{
public:
	void handle(const std::shared_ptr<Session>& session) override;
};

class S2C_SetNicknameResultHandler : public S2C_SetNicknameResult
{
public:
	void handle(const std::shared_ptr<Session>& session) override;
};
