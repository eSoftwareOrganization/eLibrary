#pragma once

#include <Core/String.hpp>

#ifdef __GNUC__
#include <cxxabi.h>
#endif
#include <exception>
#include <sstream>

namespace eLibrary {
    class Exception : public Object, public std::exception {
    private:
        std::string ErrorDetail;
        String ErrorMessage;
    public:
        explicit Exception(const String &Message) noexcept : ErrorMessage(Message) {
            std::stringstream ObjectStream;
#ifdef __GNUC__
            ObjectStream << abi::__cxa_demangle(typeid(*this).name(), nullptr, nullptr, nullptr) << ' ' << ErrorMessage.toU8String();
#else
            ObjectStream << typeid(*this).name() << ' ' << ErrorMessage.toU8String();
#endif
            ErrorDetail = ObjectStream.str();
        }

        String toString() const noexcept override {
            return ErrorDetail;
        }

        const char *what() const noexcept override {
            return ErrorDetail.data();
        }
    };

    class IOException final : public Exception {
    public:
        explicit IOException(const String &Message) noexcept : Exception(Message) {}
    };
}
