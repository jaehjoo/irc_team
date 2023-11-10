/**
 * 테스트 메인
 * exception 디렉토리에서 "g++ *.cpp ./util/\*.cpp"로 실행해보시면 됩니다.
*/

#include "ExceptionCode.hpp"
#include "ExceptionThrower.hpp"
#include "./util/Printer.hpp"

#define THROW_IF_SYS_FAIL(condition, code) \
        if(condition == -1) { ExceptionThrower::throwAsRuntimeException(code); }

int returnsSysFail() {
    return -1;
}

int main(void) {
    std::string message = "THIS MESSAGE WILL BE PRINTED AS RED";
    Printer::println(message, Printer::RED);
    Printer::println("프린트 할 일이 있다면 이렇게 사용하는 건 어떠신가요?");

    THROW_IF_SYS_FAIL(returnsSysFail(), ExceptionCode(ExceptionCode::SYSTEM_CALL_FAILURE, "SOCKET CREATION FAILED"));
}