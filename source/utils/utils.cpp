#include "../../include/utils.hpp"

const std::string suffix = "\r\n";

std::string const error::ERR_NEEDMOREPARAMS(std::string const& server_host, std::string const& command) {
	return ":" + server_host + " 461 " + command + " :Not enough parameters" + suffix;
}

std::string const error::ERR_ALREADYREGISTERED(std::string const& server_host) {
	return ":" + server_host + " 462 :You may not reregister" + suffix;
}

std::string const error::ERR_PASSWDMISMATCH(std::string const& server_host) {
	return ":" + server_host + " 464 :Password incorrect" + suffix;
}

std::string const error::ERR_NONICKNAMEGIVEN(std::string const& server_host) {
	return ":" + server_host + " 431 :No nickname given" + suffix;
}

std::string const error::ERR_NICKNAMEINUSE(std::string const& server_host, std::string const& nick) {
	return ":" + server_host + " 433 " + nick + " :Nickname is already in use" + suffix;
}

std::string const error::ERR_ERRONEUSNICKNAME(std::string const& server_host, std::string const& nick) {
	return ":" + server_host + " 432 " + nick + " :Erroneus nickname" + suffix;
}

std::string const error::ERR_NOTREGISTERED(std::string const& server_host, std::string const& reason) {
	return ":" + server_host + " 451 :" + reason + suffix;
}

std::string const error::ERR_UNKNOWNCOMMAND(std::string const& server_host, std::string const& command) {
	return ":" + server_host + " 421 " + command + " :Unknown Command" + suffix;
}

std::string const reply::RPL_WELCOME(std::string const& server_host, std::string const& nick, std::string const& user, std::string const& host) {
	return ":" + server_host + " 001 " + nick + " :Welcome to the Internet Relay Network " + nick + "!" + user + "@" + host + suffix;
}

std::string const reply::RPL_YOURHOST(std::string const& server_host, std::string const& nick, std::string const& version) {
	return ":" + server_host + " 002 " + nick + " :Your host is " + server_host + ", running version " + version + suffix;
}

std::string const reply::RPL_CREATED(std::string const& server_host, std::string const& nick, std::string const& date) {
	return ":" + server_host + " 003 " + nick + " :This server was created " + date + suffix;
}

std::string const reply::RPL_MYINFO(std::string const& server_host, std::string const& nick, std::string const& version, std::string const& usermode, std::string const& chanmode) {
	return ":" + server_host + " 004 " + nick + " :" + server_host + " " + version + " " + usermode + " " + chanmode + suffix;
}

std::string const reply::RPL_ISUPPORT(std::string const& server_host, std::string const& nick) {
	return ":" + server_host + " 005 " + nick + " :CASEMAPPING=ascii CHANMODES=i,t,k,o,l CHANTYPES=&# CHARSET=ascii MASCHANNELS=10 MAXNICKLEN=9" + suffix; 
}

std::string const reply::RPL_MOTDSTART(std::string const& server_host, std::string const& nick) {
	return ":" + server_host + " 375 " + nick + " :-" + server_host + " Message of the day - " + suffix;
}

std::string const reply::RPL_MOTD(std::string const& server_host, std::string const& nick, std::string const& line) {
	return ":" + server_host + " 372 " + nick + " :" + line + suffix;
}

std::string const reply::RPL_ENDOFMOTD(std::string const& server_host, std::string const& nick) {
	return ":" + server_host + " 376 " + nick + " :End of /MOTD command." + suffix;
}

time_t getCurTime() {
	return time(NULL);
}

std::string getStringTime(time_t const& time) {
	char buf[50];
	struct tm* tm;

	memset(buf, 0, sizeof(buf));
	tm = localtime(&time);
	strftime(buf, 50, "%c", tm);
	return buf;
}
