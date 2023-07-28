#pragma once

#include <Core/String.hpp>

#ifdef __GNUC__
#include <cxxabi.h>
#endif
#include <exception>
#include <sstream>

namespace eLibrary::Core {
    /**
     * Support for handling exceptions
     */
    class Exception : public Object, public std::exception {
    private:
        std::string ExceptionDetail;
        String ExceptionMessage;
    public:
        explicit Exception(const String &ExceptionMessageSource) noexcept : ExceptionMessage(ExceptionMessageSource) {
            std::stringstream ObjectStream;
            ObjectStream <<
#ifdef __GNUC__
            abi::__cxa_demangle(
#endif
            typeid(*this).name()
#ifdef __GNUC__
            , nullptr, nullptr, nullptr)
#endif
            << ' ' << ExceptionMessageSource.toU8String();
            ExceptionDetail = ObjectStream.str();
        }

        String toString() const noexcept override {
            return {ExceptionDetail};
        }

        const char *what() const noexcept override {
            return ExceptionDetail.data();
        }
    };

    class ArithmeticException final : public Exception {
    public:
        explicit ArithmeticException(const String &ExceptionMessage) noexcept : Exception(ExceptionMessage) {};
    };

    class InterruptedException final : public Exception {
    public:
        explicit InterruptedException(const String &ExceptionMessage) noexcept : Exception(ExceptionMessage) {}
    };

    class RuntimeException final : public Exception {
    public:
        explicit RuntimeException(const String &ExceptionMessage) noexcept : Exception(ExceptionMessage) {}
    };
}
