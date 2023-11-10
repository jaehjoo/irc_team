#include "../include/Server.hpp"
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
}

Server::~Server() {
	for (cltmap::iterator it = clients.begin(); it != clients.end(); it++)
		delete it->second;
	for (chlmap::iterator it = channels.begin(); it != channels.end(); it++)
		delete it->second;
	for (pollvec::iterator it = conn_fds.begin(); it < conn_fds.end(); it++)
		close(it->fd);
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
	struct pollfd tmp = {this->serv_sock, POLL_IN, 0};
	this->conn_fds.push_back(tmp);

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
	std::cout << "[" << getStringTime(getCurTime()) << "] server start!" << std::endl;
	while (running) {
		poll(this->conn_fds.begin().base(), (nfds_t)this->conn_fds.size(), 3000);
		for (pollvec::iterator it = this->conn_fds.begin(); it < this->conn_fds.end(); it++) {
			if (it->revents & POLL_HUP) {
				if (it->fd == serv_sock) {
					running = false;
					break;
				}
				else {
					delClient(it);
				}
			} else if (it->revents & POLL_IN) {
				if (it->fd == serv_sock) {
					addClient(it);
				}
				else {
					read_message(it);
				}
			} else if (it->revents & POLL_OUT) {
				send_message(it->fd);
			}
		}
		pingLoop();
	}
}

void Server::addClient(pollvec::iterator& it) {
	int clnt_sock;
	struct sockaddr_in clnt_adr;
	socklen_t clnt_sz;
	pollfd tmp;
	pollvec::iterator::difference_type it_diff;

	memset(&tmp, 0, sizeof(tmp));
	it_diff = std::distance(this->conn_fds.begin(), it);
	clnt_sz = sizeof(clnt_adr);
	if ((clnt_sock = accept(it->fd, (struct sockaddr*)&clnt_adr, &clnt_sz)) == -1)
		throw std::runtime_error("Error : accept!()");
	tmp = (struct pollfd){clnt_sock, POLL_IN, 0};
	this->conn_fds.push_back(tmp);
	it = this->conn_fds.begin() + it_diff;
	this->clients.insert(std::make_pair(clnt_sock, new Client(clnt_sock, clnt_adr.sin_addr)));
	this->read_buf.insert(std::make_pair(clnt_sock, ""));
	this->send_buf.insert(std::make_pair(clnt_sock, ""));
	fcntl(clnt_sock, F_SETFL, O_NONBLOCK);
}

void Server::delClient(pollvec::iterator& it) {
	if (this->op == this->clients[it->fd])
		this->op = NULL;
	delete this->clients[it->fd];
	this->read_buf.erase(it->fd);
	this->send_buf.erase(it->fd);
	close(it->fd);
	it = this->conn_fds.erase(it);
	this->clients.erase(it->fd);
	// Channel에서 제거하는 부분 추가로 필요
}

void Server::addChannel(std::string ch_name) {

}

void Server::delChannel(std::string ch_name) {

}

void Server::pingLoop() {

}

void Server::read_message(pollvec::iterator it) {
	char buf[BUF_SIZE + 1];
	std::string tmp;
	std::string message = "";
	int byte = 0;
	size_t size = 0;
	bool first = true;
	int stat;

	memset(buf, 0, sizeof(buf));
	if ((byte = recv(it->fd, buf, BUF_SIZE, 0)) == -1)
		return ;
	if (byte == 0)
		return delClient(it);
	tmp = buf;
	while ((size = tmp.find("\r\n")) != std::string::npos) {
		if (first) {
			message = this->read_buf[it->fd];
			read_buf[it->fd] = "";
			first = false;
		}
		message += tmp.substr(0, size + 2);
		tmp = tmp.substr(size + 2, tmp.size());
		switch (this->handler->parsMessage(message)) {
			// 각 case에 대한 CommandHandle 멤버 함수 연계
			case IS_PASS:
				handler->pass(*this->clients[it->fd], this->password);
				break;
			case IS_NICK:
				handler->nick(*this->clients[it->fd], this->clients);
				break;
			case IS_USER:
				handler->user(*this->clients[it->fd]);
				break;
			case IS_NOT_ORDER:
				break;
		};
		message = "";
	}
	this->read_buf[it->fd] += tmp;
}

void Server::send_message(int fd) {
	if (send_buf[fd] != "") {
		size_t size = 0;
		std::string mes = 0;

		mes = send_buf[fd];
		size = send(fd, &mes[0], mes.size(), 0);
		if (size == -1)
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
		size = send(fd, &mes[0], mes.size(), 0);
		if (size == -1)
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
