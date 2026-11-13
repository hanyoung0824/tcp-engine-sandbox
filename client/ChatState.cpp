#include "ChatState.hpp"

#include <utility>

void ChatState::setNickname(std::string nickname)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_nickname = std::move(nickname);
}

bool ChatState::isEntered() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return !m_nickname.empty();
}

void ChatState::clear()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_nickname.clear();
}
