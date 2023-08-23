#pragma once

#include <Core/String.hpp>

#if eLibraryCompiler(GNU)
#include <cxxabi.h>
#endif
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

        String toString() const noexcept override {
            if (ExceptionDetail.empty()) {
                ExceptionDetail = String(
#if eLibraryCompiler(GNU)
                        abi::__cxa_demangle(
#endif
                                typeid(*this).name()
#if eLibraryCompiler(GNU)
                                , nullptr, nullptr, nullptr)
#endif
                ).doConcat(u' ').doConcat(ExceptionMessage).toU8String();
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
        explicit ArithmeticException(const String &ExceptionMessage) noexcept : Exception(ExceptionMessage) {};
    };

    class IndexException final : public Exception {
    public:
        explicit IndexException(const String &ExceptionMessage) noexcept : Exception(ExceptionMessage) {};
    };

    class InterruptedException final : public Exception {
    public:
        explicit InterruptedException(const String &ExceptionMessage) noexcept : Exception(ExceptionMessage) {}
    };

    class RuntimeException final : public Exception {
    public:
        explicit RuntimeException(const String &ExceptionMessage) noexcept : Exception(ExceptionMessage) {}
    };

    class TypeException final : public Exception {
    public:
        explicit TypeException(const String &ExceptionMessage) noexcept : Exception(ExceptionMessage) {}
    };
}
