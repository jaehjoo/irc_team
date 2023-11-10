#ifndef PRINTER_HPP
#define PRINTER_HPP

/**
 * std::cout.. 과 같은 보일러 플레이트를 덜어내고, 색깔을 입혀 프린트하는 클래스입니다.
 */

#include <iostream>

enum Color {
    RESET = 0,
    RED = 31,
    GREEN = 32,
    YELLOW = 33,
    BLUE = 34,
    MAGENTA = 35,
    CYAN = 36,
    WHITE = 37
};

class Printer {
    private:
        Printer();

    public:

        static void println(std::string message);
        static void println(std::string message, Color color);

        ~Printer();
};

#endif
