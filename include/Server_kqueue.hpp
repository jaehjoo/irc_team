#ifndef _SERVER_HPP_
# define _SERVER_HPP_

# define MAX_LISTEN 500
# define MAX_BUFFER 1000

# include <iostream>
# include <string>

# include <sys/types.h>
# include <sys/socket.h>
# include <sys/event.h>
# include <sys/time.h>
# include <arpa/inet.h>
# include <vector>
# include <map>
# include <unistd.h>

class Client;
class Channel;
class CommandHandle;

# include "Client.hpp"
# include "Channel.hpp"
# include "CommandHandle.hpp"
# include "utils.hpp"

/*
	server가 하는 일
	1. client의 연결, 연결 해제, 연결 오류 처리 등. 전반적인 네트워크 연결을 담당한다.
		a. 소켓을 보유
		b. 클라이언트 목록 보유
		c. IRC 서버 운영자
	2. 채널의 생성, 해제 관리
		a. 채널 목록 보유
	3. 메세지 파싱
	4. IRC 프로토콜 명령어를 연결
		a. 명령어 핸들링 보유
		b. 명령어 핸들링 결과 나오는 숫적 응답 및 오류 처리
	5. 클라이언트에 주기적으로 핑 보내기
	6. 시그널 핸들링
*/
class Server {
	typedef std::vector<struct kevent> kquvec;
	typedef std::map<int, std::string> fdmap;
	typedef std::map<int, Client*> cltmap;
	typedef std::map<std::string, Channel*> chlmap;
private:
	// irc 서버로서 가져야 할 기본 정보들
	int serv_sock;
	struct sockaddr_in serv_addr;
	std::string password;
	std::string op_name;
	std::string op_password;
	std::string host;
	int port;
	Client* op;
	time_t start_time;

	// 서버 종료가 필요할 때, 플래그를 올려줄 함수
	bool running;

    // 메세지 구문 분석이 필요할 때 사용할 클래스
	CommandHandle* handler;

	// 소켓 이용 통신 및 명령어 집행 시 필요
	int kq;
	kquvec conn_fds;
	cltmap clients;
	chlmap channels;

	// 버퍼
	fdmap read_buf;
	fdmap send_buf;

	// 사용 안 함
	Server();
	Server& operator=(Server const& ref);
public:
	// 생성자와 파괴자
	Server(std::string port, std::string password);
	~Server();

	// 소켓 연결 및 통신
	void init();

	// 소켓을 연 후에 계속 돌아가는 부분
	void loop();

	// event 넣기
	void push_events(kquvec& list, uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void* udata);

	// 클라이언트 생성 및 삭제
	void addClient(int fd);
	void delClient(int fd);

	// 채널 생성 및 삭제
	void addChannel(std::string ch_name);
	void delChannel(std::string ch_name);

	// 클라이언트와 연결 확인
	void pingLoop();

	// I/O
	void read_message(int fd, intptr_t data);
	void send_message(int fd);
	void send_message(int fd, std::string message);

	// private 변수 내용물 받기
	int getServerSocket() const;
	std::string const& getHost() const;
	struct sockaddr_in const& getServAddr() const;
	int getPort() const;
	std::string getPassword() const;
	Client& getOp() const;
	time_t const& getServStartTime() const;

	// 명령어 실행을 위한 부속 함수들
	// 에러 처리
};

#endif
