#ifndef _UTILS_HPP_
# define _UTILS_HPP_

// 서버에서 명령어를 확인하는 부분
# define IS_PASS 1
# define IS_NICK 2
# define IS_USER 4
# define IS_NOT_ORDER 421



# define BUF_SIZE 600 // read, write 버퍼 상한
# define CONNECT 1000 // 서버 접속자 수 상한
# define PINGTIME 5000000 // 5 sec
# define USER_LIMIT_PER_CHANNEL 10 // 채널 접속자 수 상한

# include <ctime>
# include <string>

/*
	유틸이 할 일
	1. 숫적 응답(RPL, ERROR)
	2. 서버에서 상황을 알려줄 프린트용 함수
	3. 프린트할 때 넣어줄 색깔 정의
	4. 
*/

const std::string suffix = "\r\n";

namespace error {
	std::string ERR_NEEDMOREPARAMS(std::string command);
	std::string ERR_ALREADYREGISTERED();
	std::string ERR_PASSWDMISMATCH();
	std::string ERR_NONICKNAMEGIVEN();
	std::string ERR_NICKNAMEINUSE(std::string nick);
	std::string ERR_ERRONEUSNICKNAME(std::string nick);
}

namespace reply {

}

#endif
