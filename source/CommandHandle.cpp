#include "../include/CommandHandle.hpp"
#include "../include/utils.hpp"

CommandHandle::CommandHandle(Server& server) : server(server) {
}

CommandHandle::~CommandHandle() {
}

static bool chk_forbidden_message(std::string const& str, std::string const& forbidden_set) {
	for (int i = 0; i < str.size(); i++) {
		for (int j = 0; j < forbidden_set.size(); j++)
			if (str[i] == forbidden_set[j])
				return false;
	}
	return true;
}

static int chk_command(std::string& command) {
	if (command == "PASS")
		return IS_PASS;
	else if (command == "NICK")
		return IS_NICK;
	else if (command == "USER")
		return IS_USER;
	// 계속 이어짐 ...
	else
		return IS_NOT_ORDER;
}

// wildcard 표현식은 포함하지 않았습니다. 나중에...
int CommandHandle::parsMessage(std::string& origin) {
	bool first = true;
	std::istringstream str;
	std::string tmp = "";

	this->mes_form.clear();
	if (origin[origin.size() - 2] != '\r' || origin[origin.size() - 1] != '\n')
		return IS_NOT_ORDER;
	str.str(origin.substr(0, origin.size() - 2));
	while (std::getline(str, tmp, ' ')) {
		if (first) {
			if (tmp.empty() || !chk_forbidden_message(tmp, "\r\n\0"))
				return IS_NOT_ORDER;
			else
				this->mes_form.push_back(tmp);
			first = false;
		}
		else {
			if (tmp.empty())
				return IS_NOT_ORDER;
			else if (tmp[0] == ':') {
				this->mes_form.push_back(tmp);
				break;
			}
			else {
				if (chk_forbidden_message(tmp, ":\r\n\0"))
					this->mes_form.push_back(tmp);
				else
					return IS_NOT_ORDER;
			}
		}
	}
	tmp = "";
	str >> tmp;
	if (tmp != "")
		mes_form[mes_form.size() - 1] += " " + tmp;
	return chk_command(this->mes_form[0]);
}

void CommandHandle::pass(Client& client, std::string& password) {
	if (this->mes_form.size() != 2) {
		server.send_message(client.getClientFd(), error::ERR_NEEDMOREPARAMS(server.getHost(), "PASS"));
	} else if (client.getPassConnect() & IS_PASS) {
		server.send_message(client.getClientFd(), error::ERR_ALREADYREGISTERED(server.getHost()));
	} else if (this->mes_form[1] != password) {
		server.send_message(client.getClientFd(), error::ERR_PASSWDMISMATCH(server.getHost()));
	} else {
		client.setPassConnect(IS_PASS);
	}
}

static bool duplicate_nick(std::map<int, Client*>& clients, std::string& nick) {
	for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); it++) {
		if (clients[it->first]->getNick() == nick)
			return true;
	}
	return false;
}

void CommandHandle::nick(Client& client, std::map<int, Client*>& clients) {
	// Nickname 충돌 오류는 어차피 서버 간 통신은 신경 쓰지 않아도 되기에 구현 안 함
	if (this->mes_form.size() != 2)
		server.send_message(client.getClientFd(), error::ERR_NONICKNAMEGIVEN(server.getHost()));
	else if (duplicate_nick(clients, mes_form[1]))
		server.send_message(client.getClientFd(), error::ERR_NICKNAMEINUSE(server.getHost(), mes_form[1]));
	else if (!chk_forbidden_message(mes_form[1], "#&:") || std::isdigit(mes_form[1][0]))
		server.send_message(client.getClientFd(), error::ERR_ERRONEUSNICKNAME(server.getHost(), mes_form[1]));
	else {
		client.setPassConnect(IS_NICK);
		if (client.getNick() != "") {
			// 별칭 변경 사항 타 클라이언트에게 알리기
		}
		client.setNick(mes_form[1]);
	}
}

void CommandHandle::user(Client& client) {
	if (this->mes_form.size() != 5)
		server.send_message(client.getClientFd(), error::ERR_NEEDMOREPARAMS(server.getHost(), "USER"));
	else if (client.getPassConnect() & IS_USER)
		server.send_message(client.getClientFd(), error::ERR_ALREADYREGISTERED(server.getHost()));
	else if (!(client.getPassConnect() & (IS_PASS | IS_NICK)))
		server.send_message(client.getClientFd(), error::ERR_NOTREGISTERED(server.getHost(), "You input pass, before It enroll User"));
	else {
		client.setPassConnect(IS_USER);
		client.setUser(mes_form[1]);
		client.setHost(mes_form[2]);
		client.setServ(mes_form[3]);
		if (mes_form[4][0] == ':')
			client.setReal(mes_form[4].substr(1, mes_form[4].size()));
		else
			client.setReal(mes_form[4]);
		if (client.getPassConnect() & IS_LOGIN) {
			time_t serv_time = server.getServStartTime();
			server.send_message(client.getClientFd(), reply::RPL_WELCOME(server.getHost(), client.getNick(), client.getUser(), client.getHost()));
			server.send_message(client.getClientFd(), reply::RPL_YOURHOST(server.getHost(), client.getNick(), "1.0"));
			server.send_message(client.getClientFd(), reply::RPL_CREATED(server.getHost(), client.getNick(), getStringTime(serv_time)));
			server.send_message(client.getClientFd(), reply::RPL_MYINFO(server.getHost(), client.getNick(), "ircserv 1.0", "x", "itkol"));
			server.send_message(client.getClientFd(), reply::RPL_ISUPPORT(server.getHost(), client.getNick()));
			motd(client);
		}
	}
}

void CommandHandle::motd(Client& client) {
	server.send_message(client.getClientFd(), reply::RPL_MOTDSTART(server.getHost(), client.getNick()));
	server.send_message(client.getClientFd(), reply::RPL_MOTD(server.getHost(), client.getNick(), "Hello! This is FT_IRC!"));
	server.send_message(client.getClientFd(), reply::RPL_ENDOFMOTD(server.getHost(), client.getNick()));
}
