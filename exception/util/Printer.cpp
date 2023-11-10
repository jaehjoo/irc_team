#include <iostream>

#include "Printer.hpp"

Printer::Printer() {
}

Printer::~Printer() {
}

void Printer::println(std::string message) {
    std::cout << message << std::endl;
}

void Printer::println(std::string message, Color color) {
    std::cout << "\x1b[" << color << "m" << message << "\x1b[0m" << std::endl;
}
