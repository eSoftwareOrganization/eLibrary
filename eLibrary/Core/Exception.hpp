#pragma once

#ifndef eLibraryHeaderCoreException
#define eLibraryHeaderCoreException

#include <Core/String.hpp>
#include <exception>

namespace eLibrary::Core {
    /**
     * Support for handling exceptions
     */
    class Exception : public Object, public std::exception {
    private:
        mutable std::string ExceptionDetail;
        String ExceptionMessage;
    public:
        explicit Exception(const String &ExceptionMessageSource) noexcept : ExceptionMessage(ExceptionMessageSource) {}

        const char *getClassName() const noexcept override {
            return "Exception";
        }

        String toString() const noexcept override {
            if (ExceptionDetail.empty()) {
                ExceptionDetail = String(getClassName()).doConcat(u' ').doConcat(ExceptionMessage).toU8String();
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
        using Exception::Exception;

        const char *getClassName() const noexcept override {
            return "ArithmeticException";
        }
    };

    class IndexException final : public Exception {
    public:
        using Exception::Exception;

        const char *getClassName() const noexcept override {
            return "IndexException";
        }
    };

    class InterruptedException final : public Exception {
    public:
        using Exception::Exception;

        const char *getClassName() const noexcept override {
            return "InterruptedException";
        }
    };

    class RuntimeException final : public Exception {
    public:
        using Exception::Exception;

        const char *getClassName() const noexcept override {
            return "RuntimeException";
        }
    };

    class TypeException final : public Exception {
    public:
        using Exception::Exception;

        const char *getClassName() const noexcept override {
            return "TypeException";
        }
    };
}

#endif
