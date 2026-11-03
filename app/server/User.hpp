#pragma once

#include "IPacket.hpp"
#include "PacketHeader.hpp"
#include "Session.hpp"

#include <memory>
#include <string>

// 접속한 유저 하나 (응용 계층).
// Session(엔진)은 연결 하나일 뿐이고, User는 그 연결 너머의 사람이다. 이 프로그램이 유저에 대해 아는 정보를 가진다.
// 지금은 이름이 "IP:포트"뿐이지만, 로그인을 붙이면 닉네임 같은 정보가 여기에 들어간다.
//
// User가 Session을 붙잡고, Session은 User를 모른다 (서로 붙잡는 순환이 없다).
// 연결이 닫히면 ChatServer::onDisconnected에서 UserManager가 목록에서 빼며 함께 정리된다.
class User
{
public:
	explicit User(std::shared_ptr<Session> session);

	// 이 유저의 연결 ID (UserManager가 찾을 때 쓴다)
	Session::SessionId getId() const { return m_session->getId(); }
	const std::string& getName() const { return m_name; }

	// 이 유저에게 packet을 보낸다.
	void send(const IPacket& packet);

	// 이미 만든 패킷 바이트를 보낸다 (여러 유저에게 같은 패킷을 보낼 때 공유한다).
	void send(std::shared_ptr<const PacketBuffer> buffer);

private:
	std::shared_ptr<Session> m_session; // 이 유저의 연결
	std::string m_name; // 보낸 사람 표시용 이름
};
