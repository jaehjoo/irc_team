#include "../include/utils.hpp"

std::string error::ERR_NEEDMOREPARAMS(std::string command) {
	return "461 " + command + " :Not enough parameters" + suffix;
}

std::string error::ERR_ALREADYREGISTERED() {
	return "462 :You may not reregister" + suffix;
}

std::string error::ERR_PASSWDMISMATCH() {
	return "464 :Password incorrect" + suffix;
}

std::string error::ERR_NONICKNAMEGIVEN() {
	return "431 :No nickname given" + suffix;
};

std::string error::ERR_NICKNAMEINUSE(std::string nick) {
	return "433 " + nick + " :Nickname is already in use" + suffix;
}

std::string error::ERR_ERRONEUSNICKNAME(std::string nick) {
	return "432 " + nick + " :Erroneus nickname" + suffix;
}
