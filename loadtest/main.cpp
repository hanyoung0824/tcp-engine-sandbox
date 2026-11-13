// 사용법: loadtest [--host 127.0.0.1] [--port 12345] [--clients 100] [--messages 50] [--interval-ms 100] [--timeout-s 60]
// 서버는 채팅, 수신 제한을 높인 설정으로 띄움: server loadtest-server.json
//
// 봇 clients개가 로그인한 뒤 각자 messages개의 채팅을 interval-ms 간격으로 보냄
// 서버는 채팅을 모두에게 보내므로 봇마다 clients * messages개를 받아야 함
// 누락이나 순서 뒤바뀜이 있으면 종료 코드 1

#include "LoadTest.hpp"

#include <charconv>
#include <iostream>
#include <optional>
#include <string_view>

namespace
{
	void printUsage()
	{
		std::cerr << "usage: loadtest [--host 127.0.0.1] [--port 12345] [--clients 100] [--messages 50]"
			" [--interval-ms 100] [--timeout-s 60]\n";
	}

	bool parsePositive(std::string_view text, int& out)
	{
		const auto [end, ec] = std::from_chars(text.data(), text.data() + text.size(), out);
		return ec == std::errc() && end == text.data() + text.size() && out > 0;
	}

	std::optional<LoadTestOptions> parseOptions(int argc, char* argv[])
	{
		LoadTestOptions options;
		for (int i = 1; i < argc; i += 2) {
			const std::string_view name = argv[i];
			if (i + 1 >= argc) {
				return std::nullopt;
			}
			const std::string_view value = argv[i + 1];

			int number = 0;
			if (name == "--host") {
				options.host = value;
			}
			else if (name == "--port" && parsePositive(value, number) && number <= 65535) {
				options.port = static_cast<unsigned short>(number);
			}
			else if (name == "--clients" && parsePositive(value, number)) {
				options.clients = number;
			}
			else if (name == "--messages" && parsePositive(value, number)) {
				options.messages = number;
			}
			else if (name == "--interval-ms" && parsePositive(value, number)) {
				options.interval = std::chrono::milliseconds(number);
			}
			else if (name == "--timeout-s" && parsePositive(value, number)) {
				options.timeout = std::chrono::seconds(number);
			}
			else {
				return std::nullopt;
			}
		}
		return options;
	}
}

int main(int argc, char* argv[])
{
	const std::optional<LoadTestOptions> options = parseOptions(argc, argv);
	if (!options) {
		printUsage();
		return 2;
	}

	LoadTest test(*options);
	return test.run();
}
