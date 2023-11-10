#include "../include/CommandHandle.hpp"
#include "../include/utils.hpp"

CommandHandle::CommandHandle(Server& server) : server(server) {
}

CommandHandle::~CommandHandle() {
}

bool CommandHandle::chkForbiddenMessage(std::string const& str, std::string const& forbidden_set) {
	for (int i = 0; i < str.size(); i++) {
		for (int j = 0; j < forbidden_set.size(); j++)
			if (str[i] == forbidden_set[j])
				return false;
	}
	return true;
}

int CommandHandle::chkCommand(std::string& command) {
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

	this->mesForm.clear();

	if (origin[origin.size() - 2] != '\r' || origin[origin.size() - 1] != '\n')
		return IS_NOT_ORDER;

	str.str(origin.substr(0, origin.size() - 2));

	while (std::getline(str, tmp, ' ')) {
		if (first) {
			if (tmp.empty() || !chkForbiddenMessage(tmp, "\r\n\0"))
				return IS_NOT_ORDER;
			else
				this->mesForm.push_back(tmp);
			first = false;
		}
		else {
			if (tmp.empty())
				return IS_NOT_ORDER;
			else if (tmp[0] == ':') {
				this->mesForm.push_back(tmp);
				break;
			}
			else {
				if (chkForbiddenMessage(tmp, ":\r\n\0"))
					this->mesForm.push_back(tmp);
				else
					return IS_NOT_ORDER;
			}
		}
	}

	tmp = "";
	str >> tmp;
	if (tmp != "")
		mesForm[mesForm.size() - 1] += " " + tmp;

	return chkCommand(this->mesForm[0]);
}

void CommandHandle::pass(Client& client, std::string& password) {
	if (this->mesForm.size() != 2) {
		server.sendMessage(client.getClientFd(), error::ERR_NEEDMOREPARAMS(server.getHost(), "PASS"));
	} else if (client.getPassConnect() & IS_PASS) {
		server.sendMessage(client.getClientFd(), error::ERR_ALREADYREGISTERED(server.getHost()));
	} else if (this->mesForm[1] != password) {
		server.sendMessage(client.getClientFd(), error::ERR_PASSWDMISMATCH(server.getHost()));
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
	if (this->mesForm.size() != 2)
		server.sendMessage(client.getClientFd(), error::ERR_NONICKNAMEGIVEN(server.getHost()));
	else if (duplicate_nick(clients, mesForm[1]))
		server.sendMessage(client.getClientFd(), error::ERR_NICKNAMEINUSE(server.getHost(), mesForm[1]));
	else if (!chkForbiddenMessage(mesForm[1], "#&:") || std::isdigit(mesForm[1][0]))
		server.sendMessage(client.getClientFd(), error::ERR_ERRONEUSNICKNAME(server.getHost(), mesForm[1]));
	else {
		client.setPassConnect(IS_NICK);
		if (client.getNick() != "") {
			// 별칭 변경 사항 타 클라이언트에게 알리기
		}
		client.setNick(mesForm[1]);
	}
}

void CommandHandle::user(Client& client) {
	if (this->mesForm.size() != 5)
		server.sendMessage(client.getClientFd(), error::ERR_NEEDMOREPARAMS(server.getHost(), "USER"));
	else if (client.getPassConnect() & IS_USER)
		server.sendMessage(client.getClientFd(), error::ERR_ALREADYREGISTERED(server.getHost()));
	else if (!(client.getPassConnect() & (IS_PASS | IS_NICK)))
		server.sendMessage(client.getClientFd(), error::ERR_NOTREGISTERED(server.getHost(), "You input pass, before It enroll User"));
	else {
		client.setPassConnect(IS_USER);
		client.setUser(mesForm[1]);
		client.setHost(mesForm[2]);
		client.setServ(mesForm[3]);
		if (mesForm[4][0] == ':')
			client.setReal(mesForm[4].substr(1, mesForm[4].size()));
		else
			client.setReal(mesForm[4]);
		if (client.getPassConnect() & IS_LOGIN) {
			time_t serv_time = server.getServStartTime();
			server.sendMessage(client.getClientFd(), reply::RPL_WELCOME(server.getHost(), client.getNick(), client.getUser(), client.getHost()));
			server.sendMessage(client.getClientFd(), reply::RPL_YOURHOST(server.getHost(), client.getNick(), "1.0"));
			server.sendMessage(client.getClientFd(), reply::RPL_CREATED(server.getHost(), client.getNick(), getStringTime(serv_time)));
			server.sendMessage(client.getClientFd(), reply::RPL_MYINFO(server.getHost(), client.getNick(), "ircserv 1.0", "x", "itkol"));
			server.sendMessage(client.getClientFd(), reply::RPL_ISUPPORT(server.getHost(), client.getNick()));
			motd(client);
		}
	}
}

void CommandHandle::motd(Client& client) {
	server.sendMessage(client.getClientFd(), reply::RPL_MOTDSTART(server.getHost(), client.getNick()));
	server.sendMessage(client.getClientFd(), reply::RPL_MOTD(server.getHost(), client.getNick(), "Hello! This is FT_IRC!"));
	server.sendMessage(client.getClientFd(), reply::RPL_ENDOFMOTD(server.getHost(), client.getNick()));
}
