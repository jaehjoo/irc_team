#include "../include/Client.hpp"

Client::Client(int fd, in_addr info) : passConnect(0), passPing(false), op(false), fd(fd), info(info), host(inet_ntoa(info)), serv(""), nick(""), real("") {
}

Client::~Client() {
	// 추가 필요
}

void Client::joinChannel(std::string, char prefix) {
	// 추가 필요
}

void Client::setPassPing(bool flag) {
	this->passPing = flag;
}

void Client::setPassConnect(int flag) {
	this->passConnect |= flag;
}

void Client::setNick(std::string nick) {
	this->nick = nick;
}

void Client::setReal(std::string real) {
	this->real = real;
}

void Client::setHost(std::string host) {
	this->host = host;
}

void Client::setUser(std::string user) {
	this->user = user;
}

void Client::setServ(std::string serv) {
	this->serv = serv;
}

void Client::setTime() {
	this->finalTime = time(NULL);
}

bool Client::getOp() const {
	return this->op;
}

int Client::getPassConnect() const {
	return this->passConnect;
}

int Client::getClientFd() const {
	return this->fd;
}

std::string const& Client::getHost() const {
	return this->host;
}

std::string const& Client::getNick() const {
	return this->nick;
}

std::string const& Client::getReal() const {
	return this->real;
}

std::string const& Client::getUser() const {
	return this->user;
}

std::string const& Client::getServ() const {
	return this->serv;
}
