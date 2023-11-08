#include "../include/CommandHandle.hpp"
#include "../include/utils.hpp"

CommandHandle::CommandHandle(Server& server) : server(server) {
}

CommandHandle::~CommandHandle() {
}

static bool chk_forbidden_message(std::string& str, std::string const& forbidden_set) {
	for (int i = 0; i < str.size(); i++) {
		for (int j = 0; j < forbidden_set.size(); j++)
			if (str[i] == forbidden_set[j])
				return false;
	}
	return true;
}

static bool chk_command(std::string& command) {
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
	int cnt = 0;
	std::istringstream str;
	std::string tmp = "";
	bool is_one = false;

	this->mes_form.clear();
	if (origin[origin.size() - 2] != '\r' || origin[origin.size() - 1] != '\n')
		return IS_NOT_ORDER;
	str.str(origin.substr(0, origin.size() - 2));
	while (std::getline(str, tmp, ' ')) {
		if (cnt == 0) {
			if (tmp.empty() || tmp[0] == ':')
				return IS_NOT_ORDER;
			else {
				if (chk_forbidden_message(tmp, "\r\n\0"))
					this->mes_form.push_back(tmp);
				else
					return IS_NOT_ORDER;
			}
		}
		else if (cnt == 1) {
			if (tmp[0] == ':') {
				cnt++;
				continue ;
			}
			else if (tmp.empty()) {
				return IS_NOT_ORDER;
			}
			else {
				if (chk_forbidden_message(tmp, "\r\n\0"))
					this->mes_form.push_back(tmp);
			}
		}
		else {
			if (tmp.empty() || (cnt != 2 && tmp[0] == ':'))
				return IS_NOT_ORDER;
			else {
				if (chk_forbidden_message(tmp, "\r\n\0"))
					this->mes_form.push_back(tmp);
				else
					return IS_NOT_ORDER;
			}
		}
		cnt++;
	}
	return chk_command(this->mes_form[0]);
}

void CommandHandle::pass(Client& client, std::string& password) {
	for (int i = 0; i < mes_form.size(); i++) {
		std::cout << i << " " << mes_form[i] << std::endl;
	}
		std::cout << std::endl;
	if (this->mes_form.size() != 2) {
		server.send_message(client.getClientFd(), error::ERR_NEEDMOREPARAMS("PASS"));
	} else if (client.getPassConnect() & IS_PASS) {
		server.send_message(client.getClientFd(), error::ERR_ALREADYREGISTERED());
	} else if (this->mes_form[1] != password) {
		server.send_message(client.getClientFd(), error::ERR_PASSWDMISMATCH());
	} else {
		client.setPassConnect(IS_PASS);
	}
}

static bool duplicate_nick(std::map<int, Client*>& clients, std::string& nick) {
	for (int i = 0; i < clients.size(); i++) {
		if (clients[i]->getNick() == nick)
			return true;
	}
	return false;
}

void CommandHandle::nick(Client& client, std::map<int, Client*>& clients) {

	// Nickname 충돌 오류는 어차피 서버 간 통신은 신경 쓰지 않아도 되기에 구현 안 함
	if (this->mes_form.size() != 2)
		server.send_message(client.getClientFd(), error::ERR_NONICKNAMEGIVEN());
	else if (duplicate_nick(clients, mes_form[1]))
		server.send_message(client.getClientFd(), error::ERR_NICKNAMEINUSE(mes_form[1]));
	else if (!chk_forbidden_message(mes_form[1], "#&:"))
		server.send_message(client.getClientFd(), error::ERR_ERRONEUSNICKNAME(mes_form[1]));
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
		server.send_message(client.getClientFd(), error::ERR_NEEDMOREPARAMS("USER"));
	// else if (mes_form[2][0] == ":")
}
