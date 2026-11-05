#include "packets/S2C_ChatNotifyHandler.hpp"

#include <iostream>

void S2C_ChatNotifyHandler::handle(const std::shared_ptr<Session>&)
{
	std::cout << '[' << sender << "] " << message << '\n' << std::flush;
}
