#ifndef EXCEPTIONCODE_HPP
#define EXCEPTIONCODE_HPP

#include <iostream>

/**
 * enum으로 관리되는 커스텀 에러 상수와 메시지를 갖는 클래스입니다.
 * 내부적으로 에러 코드와 에러 메시지의 일관성을 위해 작성되었습니다.
 * ExceptionThrower에게 전달되어 처리됩니다.
*/
class ExceptionCode {
    private:
        const int code;
        const std::string& message;

    public:
        enum code{
          SYSTEM_CALL_FAILURE = 0,  
        };

        ExceptionCode(const int code, const std::string& message);
        ~ExceptionCode();

        int getCode() const;
        std::string getCodeAsString() const;
        std::string getMessage() const;
};


#endif
