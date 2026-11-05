// 사용법: client [host] [port]   (기본값 127.0.0.1 12345)

#include "ChatClient.hpp"

#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
	const std::string host = argc > 1 ? argv[1] : "127.0.0.1";
	const unsigned short port = argc > 2 ? static_cast<unsigned short>(std::stoi(argv[2])) : 12345;

	const int result = ChatClient().run(host, port);
	if (result == 0) {
		std::cout << "Disconnected\n";
	}
	return result;
}
