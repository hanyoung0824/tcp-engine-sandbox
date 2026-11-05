#include "ChatServer.hpp"

int main()
{
	constexpr unsigned short port = 12345;
	return ChatServer().run(port);
}
