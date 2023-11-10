#ifndef EXCEPTIONTHROWER_HPP
#define EXCEPTIONTHROWER_HPP

#include <iostream>

#include "ExceptionCode.hpp"
#include "Printer.hpp"

/**
 * 프로그램에서 전역적으로 사용하는 Exception을 처리하는 정적 클래스입니다.
*/

class ExceptionThrower {
    private:
        ExceptionThrower();

    public:
        ~ExceptionThrower();
        static void	throwAsRuntimeException(ExceptionCode code);
};

#endif
