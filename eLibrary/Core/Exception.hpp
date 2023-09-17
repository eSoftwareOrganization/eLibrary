#pragma once

#include <Core/String.hpp>
#include <exception>
#include <source_location>

namespace eLibrary::Core {
    /**
     * Support for handling exceptions
     */
    class Exception : public Object, public std::exception {
    private:
        mutable std::string ExceptionDetail;
        String ExceptionMessage;
    public:
        doDefineClassMethod(Exception)

        explicit Exception(const String &ExceptionMessageSource) noexcept : ExceptionMessage(ExceptionMessageSource) {}

        String toString() const noexcept override {
            if (ExceptionDetail.empty()) {
                ExceptionDetail = String(getClass().getClassName()).doConcat(u' ').doConcat(ExceptionMessage).toU8String();
            }
            return {ExceptionDetail};
        }

        const char *what() const noexcept override {
            toString();
            return ExceptionDetail.data();
        }
    };

    class ArithmeticException final : public Exception {
    public:
        doDefineClassMethod(ArithmeticException)

        using Exception::Exception;
    };

    class IndexException final : public Exception {
    public:
        doDefineClassMethod(IndexException)

        using Exception::Exception;
    };

    class InterruptedException final : public Exception {
    public:
        doDefineClassMethod(InterruptedException)

        using Exception::Exception;
    };

    class RuntimeException final : public Exception {
    public:
        doDefineClassMethod(RuntimeException)

        using Exception::Exception;
    };

    class TypeException final : public Exception {
    public:
        doDefineClassMethod(TypeException)

        using Exception::Exception;
    };
}
