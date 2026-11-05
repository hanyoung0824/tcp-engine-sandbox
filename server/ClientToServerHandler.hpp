#pragma once

#include "ClientToServer.hpp"

// 핸들러 이름은 <패킷 이름>Handler로 짓고 handle()을 오버라이드한 뒤 ChatServer 생성자에서 registerPacket<T>()로 등록한다.
class C2S_ChatHandler : public C2S_Chat
{
public:
	void handle(const std::shared_ptr<Session>& session) override;
};

class C2S_SetNicknameHandler : public C2S_SetNickname
{
public:
	void handle(const std::shared_ptr<Session>& session) override;
};
