#pragma once

#include <mutex>
#include <string>

// 닉네임은 네트워크 스레드가 쓰고 메인 스레드가 읽으므로 잠금으로 보호
class ChatState
{
public:
	ChatState() = default;

	ChatState(const ChatState&) = delete;
	ChatState& operator=(const ChatState&) = delete;

	void setNickname(std::string nickname);

	bool isEntered() const;

	// 세션을 잃어 다시 로그인해야 함
	void clear();

private:
	mutable std::mutex m_mutex;
	std::string m_nickname;
};
