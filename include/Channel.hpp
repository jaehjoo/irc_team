#ifndef _CHANNEL_HPP_
# define _CHANNEL_HPP_

# include <map>
# include <string>

# include "utils.hpp"

/*
	Channel이 하는 일
	1. 단일 채널에 필요한 변수 보유
		a. 채널 운영자 client 포인터
		b. 채널 소속 인원 목록
		c. ban 목록... 근데 루프백 IP면 이게 소용이 있나?
		d. 채널 모드 플래그
	2. 위 내용물을 볼 수 있는 getter 함수
	3. 채널 내 클라이언트 제거, 추가 함수
*/

class Client;

class Channel {
	typedef std::map<int, std::string> cltmap;
private:
	// 채널 운영자
	Client* chanop;
	// 채널 가입자 명단
	cltmap user_list;
	// 가입자수 상한
	int user_limit;
	// 채널 기호
	char prefix;
	// 채널 모드(비트 마스킹)
	int mode;
	// 채널 주제
	std::string topic;
	// 채널 비밀번호
	std::string password;
public:
};

#endif
