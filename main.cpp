#include <iostream>
#include "Server.hpp"

int main(int ac, char* av[]) {
	if (ac != 3) {
		std::cerr << "Error : ./ircserv [port] [password]" << std::endl;
		return 1;
	}
	try {
		Server irc_serv(av[1], av[2]);
		irc_serv.init();
		irc_serv.loop();
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
}
