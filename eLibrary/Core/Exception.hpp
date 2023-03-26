#pragma once

#include <Core/String.hpp>

#ifdef __GNUC__
#include <cxxabi.h>
#endif
#include <exception>
#include <sstream>

namespace eLibrary::Core {
    class Exception : public Object, public std::exception {
    private:
        std::string ErrorDetail;
        String ErrorMessage;
    public:
        explicit Exception(const String &ErrorMessageSource) noexcept : ErrorMessage(ErrorMessageSource) {
            std::stringstream ObjectStream;
            ObjectStream <<
#ifdef __GNUC__
            abi::__cxa_demangle(
#endif
            typeid(*this).name()
#ifdef __GNUC__
            , nullptr, nullptr, nullptr)
#endif
            << ' ' << ErrorMessage.toU8String();
            ErrorDetail = ObjectStream.str();
        }

        String toString() const noexcept override {
            return ErrorDetail;
        }

        const char *what() const noexcept override {
            return ErrorDetail.data();
        }
    };

    class ArithmeticException final : public Exception {
    public:
        explicit ArithmeticException(const String &ExceptionMessage) noexcept : Exception(ExceptionMessage) {};
    };
}
