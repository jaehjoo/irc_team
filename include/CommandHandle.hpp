#ifndef _COMMANDHANDLE_HPP_
# define _COMMANDHANDLE_HPP_

/*
	CommandHandle이 하는 일
	1. Server에게 명령어를 전달 받고 명령어를 대리 실행
*/

# include <vector>
# include <string>
# include <sstream>
# include <map>

class Server;
class Client;
class Channel;

# include "Client.hpp"
# include "Server.hpp"
# include "utils.hpp"

class CommandHandle {
	typedef std::vector<std::string> mesvec;
private:
	mesvec mesForm;
	Server& server;

	void motd(Client& client);

	bool chkForbiddenMessage(std::string const& str, std::string const& forbidden_set);
	int chkCommand(std::string& command);
public:
	CommandHandle(Server& server);
	~CommandHandle();

	// message를 구문 분석하고 map에 채워 넣음
	// message 정규 표현식만 맞는 지 확인함
	int parsMessage(std::string& origin);

	/*
		명령어는 각 명령어에 정해진 규칙대로 구문을 분석하고
		오류와 응답을 집행한다
	*/
	void pass(Client& client, std::string& password);
	void nick(Client& client, std::map<int, Client*>& clients);
	void user(Client& client);
	void quit(Client& client);

	// time 최신화 용
	void ping(Client& client);
	void pong(Client& client);

	// 명령을 수행하지만 서버에서 별도로 확인이 필요 없음
	void mode(Client& client, Channel* channel);
	void privmsg(Client& client, std::map<int, Channel*>& chl_list);
	void notice();

	// 채널 전용 명령어들
	void part(Client& client, std::map<std::string, Channel*>& chl_list);
	void join(Client& client, std::map<std::string, Channel*>& chl_list);
	void kick(Client& client, std::map<int, Client*> clt_list, Channel* channel);
	void topic(Client& client, Channel* channel);
	void invite(Client& client, std::map<int, Client*>& clt_list, Channel* channel);
};

#endif
