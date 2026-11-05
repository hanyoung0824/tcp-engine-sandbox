#pragma once

#include "Server.hpp"

#include <memory>

class ChatServer : public Server
{
public:
	ChatServer();

private:
	bool onStart() override;
	void onStop() override;

	void onConnected(const std::shared_ptr<Session>& session) override;

	void onDisconnected(const std::shared_ptr<Session>& session) override;
};
