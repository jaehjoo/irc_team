#include <iostream>

#include "ExceptionCode.hpp"

ExceptionCode::ExceptionCode(int code, const std::string& message) : code(code), message(message) {
}

ExceptionCode::~ExceptionCode() {
}

int ExceptionCode::getCode() const {
    return this->code;
}

std::string ExceptionCode::getCodeAsString() const {
    switch(this->code) {
        case ExceptionCode::SYSTEM_CALL_FAILURE:
            return "SYSTEM_CALL_FAILURE";
        default:
            return "UNKNOWN";
    }
}

std::string ExceptionCode::getMessage() const {
    return this->message;
}