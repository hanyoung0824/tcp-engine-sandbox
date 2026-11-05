#include "packets/ChatNotifyPacket.hpp"

#include <iostream>

void ChatNotifyPacket::handle(const std::shared_ptr<Session>&)
{
	std::cout << '[' << sender << "] " << message << '\n' << std::flush;
}
