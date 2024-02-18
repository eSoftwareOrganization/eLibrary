#pragma once

#ifndef eLibraryHeaderIOLibrary
#define eLibraryHeaderIOLibrary

#include <IO/IOException.hpp>
#if eLibrarySystem(Windows)
#include <libloaderapi.h>
#else
#include <dlfcn.h>
#endif

namespace eLibrary::IO {
#if eLibrarySystem(Windows)
    typedef HMODULE LibraryHandleType;
#else
    typedef void *LibraryHandleType;
#endif

    /**
     * Support for loading libraries at runtime
     */
    class Library final : public Object, public NonCopyable {
    private:
        LibraryHandleType LibraryHandle;

        constexpr Library(LibraryHandleType LibraryHandleSource) noexcept : LibraryHandle(LibraryHandleSource) {}
    public:
        ~Library() noexcept {
            if (LibraryHandle) {
#if eLibrarySystem(Windows)
                ::FreeLibrary(LibraryHandle);
#else
                ::dlclose(LibraryHandle);
#endif
                LibraryHandle = nullptr;
            }
        }

        static Library doOpen(const String &LibraryPath) {
            LibraryHandleType LibraryHandle =
#if eLibrarySystem(Windows)
                ::LoadLibraryExA(LibraryPath.toU8String().c_str(), nullptr, 0);
#else
                ::dlopen(LibraryPath.toU8String().c_str(), RTLD_LAZY);
#endif
            if (!LibraryHandle) [[unlikely]] doThrowChecked(IOException(u"Library::Library(const String&)"_S));
            return {LibraryHandle};
        }

        template<typename F>
        F getSymbol(const String &LibrarySymbol) const noexcept {
            return reinterpret_cast<F>(
#if eLibrarySystem(Windows)
                ::GetProcAddress(LibraryHandle, LibrarySymbol.toU8String().c_str())
#else
                ::dlsym(LibraryHandle, LibrarySymbol.toU8String().c_str())
#endif
            );
        }
    };
}

#endif
