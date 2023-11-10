#ifndef _UTILS_HPP_
# define _UTILS_HPP_

// 서버에서 명령어를 확인하는 부분
# define IS_PASS 1
# define IS_NICK 2
# define IS_USER 4
# define IS_LOGIN 7
# define IS_NOT_ORDER 421



# define BUF_SIZE 4096 // read, write 버퍼 상한
# define CONNECT 1000 // 서버 접속자 수 상한
# define PINGTIME 5000000 // 5 sec
# define USER_LIMIT_PER_CHANNEL 10 // 채널 접속자 수 상한, 채널 옵션 커지면 적용
# define CHANNEL_LIMIT_PER_USER 10 // 접속자 참가 채널 수 상한, 처음부터 적용
# define MAX_CHANNEL 30 // 서버가 최대로 보유할 수 있는 채널 상한
# define USERNICK_LEN 9 // 사용자 별칭의 최대 길이(RFC 1459)
# define CHANNELNAME_LEN 200 // 채널 이름 최대 길이(RFC 1459)

# include <ctime>
# include <string>

/*
	유틸이 할 일
	1. 숫적 응답(RPL, ERROR)
	2. 서버에서 상황을 알려줄 프린트용 함수
	3. 프린트할 때 넣어줄 색깔 정의
	4. 시간 계산 함수 
*/

// 숫적 응답(numeric reply) 중, 에러 담당
namespace error {
	std::string const ERR_NEEDMOREPARAMS(std::string const& server_host, std::string const& command);
	std::string const ERR_ALREADYREGISTERED(std::string const& server_host);
	std::string const ERR_PASSWDMISMATCH(std::string const& server_host);
	std::string const ERR_NONICKNAMEGIVEN(std::string const& server_host);
	std::string const ERR_NICKNAMEINUSE(std::string const& server_host, std::string const& nick);
	std::string const ERR_ERRONEUSNICKNAME(std::string const& server_host, std::string const& nick);
	std::string const ERR_NOTREGISTERED(std::string const& server_host, std::string const& reason);
}

// 숫적 응답(numeric reply) 중, 정상 응답 담당
namespace reply {
	std::string const RPL_WELCOME(std::string const& server_host, std::string const& nick, std::string const& user, std::string const& host);
	std::string const RPL_YOURHOST(std::string const& server_host, std::string const& nick, std::string const& version);
	std::string const RPL_CREATED(std::string const& server_host, std::string const& nick, std::string const& date);
	std::string const RPL_MYINFO(std::string const& server_host, std::string const& nick, std::string const& version, std::string const& usermode, std::string const& chanmode);
	std::string const RPL_ISUPPORT(std::string const& server_host, std::string const& nick);
	std::string const RPL_MOTDSTART(std::string const& server_host, std::string const& nick);
	std::string const RPL_MOTD(std::string const& server_host, std::string const& nick, std::string const& line);
	std::string const RPL_ENDOFMOTD(std::string const& server_host, std::string const& nick);
}

// 시간 계산 함수
time_t getCurTime();
std::string getStringTime(time_t const& time);

#endif
