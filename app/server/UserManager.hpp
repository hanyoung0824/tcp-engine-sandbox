#pragma once

#include "IPacket.hpp"
#include "Session.hpp"
#include "User.hpp"

#include <memory>
#include <unordered_map>

// 접속한 유저 목록 (응용 계층의 싱글톤).
// 응용은 엔진의 세션 목록(SessionManager) 대신 이 목록으로 유저를 찾고 패킷을 보낸다.
// ChatServer가 onConnected에서 add(), onDisconnected에서 remove()를 호출한다.
//
// 지금은 io_context.run()을 실행하는 스레드 하나에서만 접근하므로 잠금(mutex)이 필요 없다.
class UserManager
{
public:
	static UserManager& instance();

	UserManager(const UserManager&) = delete;
	UserManager& operator=(const UserManager&) = delete;

	// 연결된 session으로 유저를 만들어 목록에 넣는다.
	std::shared_ptr<User> add(const std::shared_ptr<Session>& session);

	// 목록에서 뺀다. 이미 없으면 아무 일도 하지 않는다.
	void remove(Session::SessionId id);

	// 연결 ID로 유저를 찾는다. 없으면 nullptr.
	std::shared_ptr<User> find(Session::SessionId id) const;

	// 모든 유저에게 packet을 보낸다. 패킷은 한 번만 만들고 모든 유저가 공유한다.
	void broadcast(const IPacket& packet);

	std::size_t size() const { return m_users.size(); }

private:
	UserManager() = default;

private:
	std::unordered_map<Session::SessionId, std::shared_ptr<User>> m_users; // 연결 ID -> 유저
};
