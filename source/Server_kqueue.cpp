#include "../include/Server_kqueue.hpp"
#include <fcntl.h>
#include <stdexcept>
#include <cstdlib>
#include <signal.h>
#include <netdb.h>

Server::Server(std::string port, std::string password) : op_name(""), op_password(""), op(NULL) {
	char* pointer;
	long strict_port;
	char hostname_buf[1024];
	struct hostent* host_struct;

	strict_port = std::strtol(port.c_str(), &pointer, 10);
	if (*pointer != 0 || strict_port <= 1000 || strict_port > 65535)
		throw std::runtime_error("Error : port is wrong");
	for (size_t i = 0; i < password.size(); i++) {
		if (password[i] == 0 || password[i] == '\r'
			|| password[i] == '\n' || password[i] == ':')
			throw std::runtime_error("Error : password is wrong");
	}
	this->password = password;
	this->port = static_cast<int>(strict_port);
	if (gethostname(hostname_buf, sizeof(hostname_buf)) == -1)
		this->host = "127.0.0.1";
	else {
		if (!(host_struct = gethostbyname(hostname_buf)))
			this->host = "127.0.0.1";
		else
			this->host = inet_ntoa(*((struct in_addr*)host_struct->h_addr_list[0]));
	}
	if ((this->kq = kqueue()) == -1)
		throw std::runtime_error("kqueue error!");
}

Server::~Server() {
	for (cltmap::iterator it = clients.begin(); it != clients.end(); it++)
		delete it->second;
	for (chlmap::iterator it = channels.begin(); it != channels.end(); it++)
		delete it->second;
	for ()
	for (kquvec::iterator it = conn_fds.begin(); it < conn_fds.end(); it++)
		close(it->ident);
	delete handler;
}

void Server::init() {
	this->serv_sock = socket(PF_INET, SOCK_STREAM, 0);

	if (serv_sock == -1)
		throw std::runtime_error("Error : server socket is wrong");

	memset(&this->serv_addr, 0, sizeof(this->serv_addr));

	this->serv_addr.sin_family = AF_INET;
	this->serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	this->serv_addr.sin_port = htons(this->port);
	push_events(this->conn_fds, this->serv_sock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);

	// signal(SIGINT, SIG_IGN);

	int yes = 1;
	setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		throw std::runtime_error("Error : bind");
	if (listen(serv_sock, CONNECT) == -1)
		throw std::runtime_error("Error : listen");
	fcntl(serv_sock, F_SETFL, O_NONBLOCK);
	this->handler = new CommandHandle(*this);
	this->running = true;
	this->start_time = getCurTime();
}

void Server::loop() {
	int new_events;
	struct kevent event_list[15];

	std::cout << "[" << getStringTime(getCurTime()) << "] server start!" << std::endl;
	while (running) {
		new_events = kevent(this->kq, &this->conn_fds[0], this->conn_fds.size(), event_list, 15, NULL);

		this->conn_fds.clear();
	
		for (int i = 0; i < new_events; i++) {
			if (event_list[i].flags & EV_ERROR) {
				if (event_list[i].ident == this->serv_sock) {
					running = false;
					break;
				}
				else {
					delClient(event_list[i].ident);
				}
			} else if (event_list[i].flags & EVFILT_READ) {
				if (event_list[i].ident == serv_sock) {
					addClient(event_list[i].ident);
				}
				else if (clients.find(event_list[i].ident) != clients.end()) {
					read_message(event_list[i].ident, event_list[i].data);
				}
			} else if (event_list[i].ident & EVFILT_WRITE) {
				if (clients.find(event_list[i].ident) != clients.end())
					send_message(event_list[i].ident);
			}
		}
		pingLoop();
	}
}

void Server::push_events(kquvec& list, uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void* udata) {
	struct kevent tmp;

	EV_SET(&tmp, ident, filter, flags, fflags, data, udata);
	list.push_back(tmp);
}

void Server::addClient(int fd) {
	int clnt_sock;
	struct sockaddr_in clnt_adr;
	socklen_t clnt_sz;

	clnt_sz = sizeof(clnt_adr);
	if ((clnt_sock = accept(fd, (struct sockaddr*)&clnt_adr, &clnt_sz)) == -1)
		throw std::runtime_error("Error : accept!()");
	push_events(this->conn_fds, clnt_sock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	push_events(this->conn_fds, clnt_sock, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
	this->clients.insert(std::make_pair(clnt_sock, new Client(clnt_sock, clnt_adr.sin_addr)));
	this->read_buf.insert(std::make_pair(clnt_sock, ""));
	this->send_buf.insert(std::make_pair(clnt_sock, ""));
	fcntl(clnt_sock, F_SETFL, O_NONBLOCK);
}

void Server::delClient(int fd) {
	if (this->op == this->clients[fd])
		this->op = NULL;
	delete this->clients[fd];
	this->read_buf.erase(fd);
	this->send_buf.erase(fd);
	close(fd);
	this->clients.erase(fd);
	// Channel에서 제거하는 부분 추가로 필요
}

void Server::addChannel(std::string ch_name) {

}

void Server::delChannel(std::string ch_name) {

}

void Server::pingLoop() {

}

void Server::read_message(int fd, intptr_t data) {
	char buf[data + 1];
	std::string tmp;
	std::string message = "";
	int byte = 0;
	size_t size = 0;
	bool first = true;
	int stat;

	memset(buf, 0, sizeof(buf));
	if ((byte = recv(fd, buf, data, 0)) == -1)
		return ;
	if (byte == 0)
		return delClient(fd);
	tmp = buf;
	while ((size = tmp.find("\r\n")) != std::string::npos) {
		if (first) {
			message = this->read_buf[fd];
			read_buf[fd] = "";
			first = false;
		}
		message += tmp.substr(0, size + 2);
		tmp = tmp.substr(size + 2, tmp.size());
		switch (this->handler->parsMessage(message)) {
			// 각 case에 대한 CommandHandle 멤버 함수 연계
			case IS_PASS:
				handler->pass(*this->clients[fd], this->password);
				break;
			case IS_NICK:
				handler->nick(*this->clients[fd], this->clients);
				break;
			case IS_USER:
				handler->user(*this->clients[fd]);
				break;
			case IS_NOT_ORDER:
				break;
		};
		message = "";
	}
	this->read_buf[fd] += tmp;
}

void Server::send_message(int fd) {
	if (send_buf[fd] != "") {
		size_t size = 0;
		std::string mes = 0;

		mes = send_buf[fd];
		if ((size = send(fd, &mes[0], mes.size(), 0)) == -1)
			return ;
		send_buf[fd] = "";
		if (size < mes.size()) {
			send_buf[fd] = mes.substr(size, mes.size());
		}
	}
}

void Server::send_message(int fd, std::string message) {
	send_buf[fd] += message;
	if (send_buf[fd] != "") {
		size_t size = 0;
		std::string mes;

		mes = send_buf[fd];
		send_buf[fd] = "";
		if ((size = send(fd, &mes[0], mes.size(), 0)) == -1)
			return ;
		if (size < mes.size()) {
			send_buf[fd] = mes.substr(size, mes.size());
		}
	}
}

int Server::getServerSocket() const {
	return this->serv_sock;
}

std::string const& Server::getHost() const {
	return this->host;
}

struct sockaddr_in const& Server::getServAddr() const {
	return this->serv_addr;
}

int Server::getPort() const {
	return this->port;
}

std::string Server::getPassword() const {
	return this->password;
}

Client& Server::getOp() const {
	return *this->op;
}

time_t const& Server::getServStartTime() const {
	return this->start_time;
}
