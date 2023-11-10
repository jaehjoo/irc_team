#include "ExceptionThrower.hpp"

ExceptionThrower::ExceptionThrower() {
}

ExceptionThrower::~ExceptionThrower() {
}

void ExceptionThrower::throwAsRuntimeException(ExceptionCode code) {
    std::string message = code.getCodeAsString() + " - " + code.getMessage();
    throw std::runtime_error(message);
}
