#pragma once

#ifndef eLibraryHeaderCoreException
#define eLibraryHeaderCoreException

#include <Core/String.hpp>
#include <exception>

namespace eLibrary::Core {
    /**
     * Support for handling exceptions
     */
    class Exception : public Object, public ::std::exception {
    private:
        mutable ::std::string ExceptionDetail;
        String ExceptionMessage;
    public:
        explicit Exception(const String &ExceptionMessageSource) noexcept : ExceptionMessage(ExceptionMessageSource) {}

        String toString() const noexcept override {
            if (ExceptionDetail.empty()) ExceptionDetail = ExceptionMessage.toU8String();
            return {ExceptionDetail};
        }

        const char *what() const noexcept override {
            toString();
            return ExceptionDetail.data();
        }
    };

    class RuntimeException : public Exception {
    public:
        using Exception::Exception;
    };

    class ArithmeticException final : public RuntimeException {
    public:
        using RuntimeException::RuntimeException;
    };

    class ConcurrentException : public Exception {
    public:
        using Exception::Exception;
    };

    class IndexException final : public Exception {
    public:
        using Exception::Exception;
    };

    class InterruptedException final : public ConcurrentException {
    public:
        using ConcurrentException::ConcurrentException;
    };

    class TypeException final : public Exception {
    public:
        using Exception::Exception;
    };

#define doThrowChecked(...) throw __VA_ARGS__
#define doThrowUnchecked(...) throw __VA_ARGS__
}

#endif
