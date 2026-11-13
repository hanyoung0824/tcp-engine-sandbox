#pragma once

#include <nlohmann/json_fwd.hpp>

#include <string>

// 사용자 한 명이 perSeconds초 동안 보낼 수 있는 채팅 수. 그만큼은 몰아서 보내도 됨
struct ChatLimitSettings
{
	double messages = 5;
	double perSeconds = 3;
};

// 채팅 서버 전용 설정. 엔진 설정(tcpengine::ServerSettings)과 따로 둠
struct ChatServerSettings
{
	// 입장, 퇴장 알림에 쓰는 보낸 사람 이름
	std::string noticeSender = "Server";
	ChatLimitSettings chatLimit;

	// 틀리면 이유를 담은 std::invalid_argument를 던짐
	void validate() const;

	// 설정 파일 전체를 받아 "app" 영역을 읽음. 영역이나 항목이 없으면 기본값
	// 실패하면 std::runtime_error (메시지에 경로 없음)
	static ChatServerSettings fromJson(const nlohmann::json& file);
};
