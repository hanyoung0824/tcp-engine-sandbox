#pragma once

#include "ChatState.hpp"
#include "ServerToClient.hpp"
#include "tcpengine/client/Client.hpp"

#include <deque>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

// 훅과 패킷 핸들러는 메인 스레드에서 실행
// 콘솔 입력은 입력 스레드가 받아 두고 onUpdate()에서 처리
class ChatClient : public tcpengine::Client
{
public:
    explicit ChatClient(const tcpengine::ClientSettings &settings);

    ~ChatClient() override;

protected:
    void onUpdate() override;

    void onConnected() override;

    void onDisconnected(const tcpengine::DisconnectInfo &info) override;

    void onSessionResumed() override;

    void onSessionLost() override;

private:
    // 빈 줄이나 입력 끝에서 멈춤. 입력 끝은 빈 줄로 넘김
    void readInput();

    std::vector<std::string> takeInputLines();

    void handleInputLine(const std::string &line);

    void handleChatNotify(const s2c::ChatNotify &packet);

    void handleSetNicknameResult(const s2c::SetNicknameResult &packet);

private:
    ChatState m_state;
    bool m_quitting = false;

    // 처음 접속한 뒤 시작. 첫 접속에 실패하면 입력을 기다리지 않고 끝내기 위함
    std::thread m_inputThread;
    std::mutex m_inputMutex;
    std::deque<std::string> m_inputLines;
};
