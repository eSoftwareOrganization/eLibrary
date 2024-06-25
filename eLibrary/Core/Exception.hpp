#pragma once

#ifndef eLibraryHeaderCoreException
#define eLibraryHeaderCoreException

#include <Core/String.hpp>
#include <exception>
#include <iostream>
#if __cpp_lib_stacktrace
#include <stacktrace>
#endif

namespace eLibrary::Core {
    /**
     * Support for handling exceptions
     */
    class Exception : public Object, public ::std::exception {
    private:
        mutable ::std::string ExceptionDetail;
        String ExceptionMessage;
#if __cpp_lib_stacktrace
        ::std::stacktrace ExceptionStacktrace;
#endif
    public:
        explicit Exception(const String &ExceptionMessageSource) noexcept : ExceptionMessage(ExceptionMessageSource) {
            ExceptionDetail = ExceptionMessage.toU8String();
#if __cpp_lib_stacktrace
            ExceptionStacktrace = ::std::stacktrace::current();
#endif
        }

        void printStackTrace() const {
            ::std::cerr << ExceptionDetail << ::std::endl;
#if __cpp_lib_stacktrace
            for (const auto &StackFrame : ExceptionStacktrace)
                ::std::cerr << "    at " << ::std::to_string(StackFrame) << ::std::endl;
#endif
        }

        String toString() const noexcept override {
            return ExceptionMessage;
        }

        const char *what() const noexcept override {
            return ExceptionDetail.c_str();
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

    class NotImplementedException final : public Exception {
    public:
        using Exception::Exception;
    };

    class TypeException final : public Exception {
    public:
        using Exception::Exception;
    };

#define doThrowChecked(ExceptionType, ...) throw ExceptionType(__VA_ARGS__)
#define doThrowUnchecked(...) throw __VA_ARGS__
}

#endif
