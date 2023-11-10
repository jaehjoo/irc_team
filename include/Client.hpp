#ifndef _CLIENT_HPP_
# define _CLIENT_HPP_

/*
	Client가 하는 일
	1. 단일 클라이언트가 가져야 할 내용을 보유
		a. 서버에서 accept하면서 얻은 전용 소켓
		b. IRC 운영자, 채널 운영자 권한 여부
		c. 소속 채널 목록
		d. nickname, hostname, servername, realname
	2. 서버 - 클라이언트 교신
		a. recv, send 버퍼를 보유
*/

# include "Server.hpp"

class Client {
	typedef std::map<std::string, bool> chllist;
private:
	// PASS, NICK, USER 전부를 거쳤는 지 검증. 비트마스킹.
	int pass_connect;

	// ping 검사. ping을 보낼 때 false로 바꾸고 pong을 받으면 true가 된다
	bool pass_ping;

	// 운영자 권한
	bool op;

	// final_ping_time
	time_t final_time;

	// client socket
	int fd;

	// client addr info
	in_addr info;

	// client hostname
	std::string host;

	// client nickname
	std::string nick;

	// client realname
	std::string real;

	// client username
	std::string user;

	// client servername
	std::string serv;

	// 현재 참여하고 있는 채널 목록
	chllist join_list;

	// 사용 안 함
	Client();
	Client(Client const& ref);
public:
	// 생성자와 파괴자
	Client(int fd, in_addr info);
	~Client();

	// 명령어 수행 함수
	void joinChannel(std::string, char prefix);

	// setter
	void setPassPing(bool flag);
	void setPassConnect(int flag);
	void setNick(std::string nick);
	void setReal(std::string real);
	void setHost(std::string host);
	void setUser(std::string user);
	void setServ(std::string serv);
	void setTime();
	// getter
	int getPassConnect() const;
	int getClientFd() const;
	bool getOp() const;
	std::string const& getHost() const;
	std::string const& getNick() const;
	std::string const& getReal() const;
	std::string const& getUser() const;
	std::string const& getServ() const;
};

#endif
