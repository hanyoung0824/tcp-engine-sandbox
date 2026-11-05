#include "ChatState.hpp"

#include <utility>

ChatState& ChatState::instance()
{
	static ChatState instance;
	return instance;
}

void ChatState::setNickname(std::string nickname)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_nickname = std::move(nickname);
}

std::string ChatState::getNickname() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_nickname;
}

bool ChatState::isEntered() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return !m_nickname.empty();
}
