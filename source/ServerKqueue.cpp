#include "../include/ServerKqueue.hpp"
#include <fcntl.h>
#include <stdexcept>
#include <cstdlib>
#include <signal.h>
#include <netdb.h>

Server::Server(std::string port, std::string password) : opName(""), opPassword(""), op(NULL) {
	char* pointer;
	long strictPort;
	char hostnameBuf[1024];
	struct hostent* hostStruct;

	strictPort = std::strtol(port.c_str(), &pointer, 10);
	if (*pointer != 0 || strictPort <= 1000 || strictPort > 65535)
		throw std::runtime_error("Error : port is wrong");
	for (size_t i = 0; i < password.size(); i++) {
		if (password[i] == 0 || password[i] == '\r'
			|| password[i] == '\n' || password[i] == ':')
			throw std::runtime_error("Error : password is wrong");
	}
	this->password = password;
	this->port = static_cast<int>(strictPort);
	if (gethostname(hostnameBuf, sizeof(hostnameBuf)) == -1)
		this->host = "127.0.0.1";
	else {
		if (!(hostStruct = gethostbyname(hostnameBuf)))
			this->host = "127.0.0.1";
		else
			this->host = inet_ntoa(*((struct in_addr*)hostStruct->h_addr_list[0]));
	}
	if ((this->kq = kqueue()) == -1)
		throw std::runtime_error("kqueue error!");
}

Server::~Server() {
	for (cltmap::iterator it = clientList.begin(); it != clientList.end(); it++)
		delete it->second;
	for (chlmap::iterator it = channelList.begin(); it != channelList.end(); it++)
		delete it->second;
	for (fdmap::iterator it = savingBufForRead.begin(); it != savingBufForRead.end(); it++)
		close(it->first);
	delete handler;
}

void Server::init() {
	int yes = 1;

	this->servSock = socket(PF_INET, SOCK_STREAM, 0);

	if (servSock == -1)
		throw std::runtime_error("Error : server socket is wrong");

	memset(&this->servAddr, 0, sizeof(this->servAddr));
	this->servAddr.sin_family = AF_INET;
	this->servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	this->servAddr.sin_port = htons(this->port);

	pushEvents(this->connectingFds, this->servSock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);

	// signal(SIGINT, SIG_IGN);

	setsockopt(servSock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

	if (bind(servSock, (struct sockaddr*)&servAddr, sizeof(servAddr)) == -1)
		throw std::runtime_error("Error : bind");
	if (listen(servSock, CONNECT) == -1)
		throw std::runtime_error("Error : listen");
	
	fcntl(servSock, F_SETFL, O_NONBLOCK);

	this->handler = new CommandHandle(*this);
	this->running = true;
	this->startTime = getCurTime();
}

void Server::loop() {
	int new_events;
	struct kevent eventList[15];

	std::cout << "[" << getStringTime(getCurTime()) << "] server start!" << std::endl;
	while (running) {
		new_events = kevent(this->kq, &this->connectingFds[0], this->connectingFds.size(), eventList, 15, NULL);

		this->connectingFds.clear();
	
		for (int i = 0; i < new_events; i++) {
			if (eventList[i].flags & EV_ERROR) {
				if (eventList[i].ident == this->servSock) {
					running = false;
					break;
				}
				else {
					delClient(eventList[i].ident);
				}
			} else if (eventList[i].flags & EVFILT_READ) {
				if (eventList[i].ident == this->servSock) {
					addClient(eventList[i].ident);
				}
				else if (clientList.find(eventList[i].ident) != clientList.end()) {
					readMessage(eventList[i].ident, eventList[i].data);
				}
			} else if (eventList[i].ident & EVFILT_WRITE) {
				if (clientList.find(eventList[i].ident) != clientList.end())
					sendMessage(eventList[i].ident);
			}
		}
		pingLoop();
	}
}

void Server::pushEvents(kquvec& list, uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void* udata) {
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
	pushEvents(this->connectingFds, clnt_sock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	pushEvents(this->connectingFds, clnt_sock, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
	this->clientList.insert(std::make_pair(clnt_sock, new Client(clnt_sock, clnt_adr.sin_addr)));
	this->savingBufForRead.insert(std::make_pair(clnt_sock, ""));
	this->savingBufForSend.insert(std::make_pair(clnt_sock, ""));
	fcntl(clnt_sock, F_SETFL, O_NONBLOCK);
}

void Server::delClient(int fd) {
	if (this->op == this->clientList[fd])
		this->op = NULL;
	delete this->clientList[fd];
	this->savingBufForRead.erase(fd);
	this->savingBufForSend.erase(fd);
	close(fd);
	this->clientList.erase(fd);
	// Channel에서 제거하는 부분 추가로 필요
}

void Server::addChannel(std::string& ch_name) {

}

void Server::delChannel(std::string& ch_name) {

}

void Server::pingLoop() {

}

void Server::readMessage(int fd, intptr_t data) {
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
			message = this->savingBufForRead[fd];
			savingBufForRead[fd] = "";
			first = false;
		}
		message += tmp.substr(0, size + 2);
		tmp = tmp.substr(size + 2, tmp.size());
		switch (this->handler->parsMessage(message)) {
			// 각 case에 대한 CommandHandle 멤버 함수 연계
			case IS_PASS:
				handler->pass(*this->clientList[fd], this->password);
				break;
			case IS_NICK:
				handler->nick(*this->clientList[fd], this->clientList);
				break;
			case IS_USER:
				handler->user(*this->clientList[fd]);
				break;
			case IS_NOT_ORDER:
				break;
		};
		message = "";
	}
	this->savingBufForRead[fd] += tmp;
}

void Server::sendMessage(int fd) {
	if (savingBufForSend[fd] != "") {
		size_t size = 0;
		std::string mes = 0;

		mes = savingBufForSend[fd];
		if ((size = send(fd, &mes[0], mes.size(), 0)) == -1)
			return ;
		savingBufForSend[fd] = "";
		if (size < mes.size()) {
			savingBufForSend[fd] = mes.substr(size, mes.size());
		}
	}
}

void Server::sendMessage(int fd, std::string message) {
	savingBufForSend[fd] += message;
	if (savingBufForSend[fd] != "") {
		size_t size = 0;
		std::string mes;

		mes = savingBufForSend[fd];
		savingBufForSend[fd] = "";
		if ((size = send(fd, &mes[0], mes.size(), 0)) == -1)
			return ;
		if (size < mes.size()) {
			savingBufForSend[fd] = mes.substr(size, mes.size());
		}
	}
}

int const& Server::getServerSocket() const {
	return this->servSock;
}

std::string const& Server::getHost() const {
	return this->host;
}

struct sockaddr_in const& Server::getServAddr() const {
	return this->servAddr;
}

int const& Server::getPort() const {
	return this->port;
}

std::string const& Server::getPassword() const {
	return this->password;
}

Client& Server::getOp() const {
	return *this->op;
}

time_t const& Server::getServStartTime() const {
	return this->startTime;
}
