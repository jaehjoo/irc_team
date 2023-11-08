#include "../include/Client.hpp"

Client::Client(int fd, in_addr info) : pass_connect(0), pass_ping(false), op(false), fd(fd), info(info), host(inet_ntoa(info)), serv_name(""), nick(""), real("") {
}

Client::~Client() {
	// 추가 필요
}

void Client::joinChannel(std::string, char prefix) {
	// 추가 필요
}

void Client::setPassPing(bool flag) {
	this->pass_ping = flag;
}

void Client::setPassConnect(int flag) {
	this->pass_connect |= flag;
}

void Client::setNick(std::string nick) {
	this->nick = nick;
}

void Client::setReal(std::string real) {
	this->real = real;
}

void Client::setServerName(std::string serv_name) {
	this->serv_name = serv_name;
}

void Client::setTime() {
	this->final_time = time(NULL);
}

std::string Client::getHost() const {
	return this->host;
}

bool Client::getOp() const {
	return this->op;
}

int Client::getPassConnect() const {
	return this->pass_connect;
}

int Client::getClientFd() const {
	return this->fd;
}

std::string Client::getNick() const {
	return this->nick;
}
