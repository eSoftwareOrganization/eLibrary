#pragma once

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

        virtual const char *getClassName() const noexcept {
            return "Core::Exception";
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
            return "Core::ArithmeticException";
        }
    };

    class IndexException final : public Exception {
    public:
        using Exception::Exception;

        const char *getClassName() const noexcept override {
            return "Core::IndexException";
        }
    };

    class InterruptedException final : public Exception {
    public:
        using Exception::Exception;

        const char *getClassName() const noexcept override {
            return "Core::InterruptedException";
        }
    };

    class RuntimeException final : public Exception {
    public:
        using Exception::Exception;

        const char *getClassName() const noexcept override {
            return "Core::RuntimeException";
        }
    };

    class TypeException final : public Exception {
    public:
        using Exception::Exception;

        const char *getClassName() const noexcept override {
            return "Core::TypeException";
        }
    };
}
