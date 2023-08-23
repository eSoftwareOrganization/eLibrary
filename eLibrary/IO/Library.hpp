#pragma once

#include <IO/Exception.hpp>
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
    class Library final : public Object {
    private:
        LibraryHandleType LibraryHandle;

        constexpr Library(LibraryHandleType LibraryHandleSource) noexcept : LibraryHandle(LibraryHandleSource) {}

        doDisableCopyAssignConstruct(Library)
    public:
        ~Library() noexcept {
            if (LibraryHandle) {
#if eLibrarySystem(Windows)
                FreeLibrary(LibraryHandle);
#else
                dlclose(LibraryHandle);
#endif
                LibraryHandle = nullptr;
            }
        }

        static Library doOpen(const String &LibraryPath) {
            LibraryHandleType LibraryHandle =
#if eLibrarySystem(Windows)
            LoadLibraryExA(LibraryPath.toU8String().c_str(), nullptr, 0);
#else
            dlopen(LibraryPath.toU8String().c_str(), RTLD_LAZY);
#endif
            if (!LibraryHandle) throw IOException(String(u"Library::Library(const String&)"));
            return {LibraryHandle};
        }

        auto getSymbol(const String &LibrarySymbol) const noexcept {
#if eLibrarySystem(Windows)
            return GetProcAddress(LibraryHandle, LibrarySymbol.toU8String().c_str());
#else
            return dlsym(LibraryHandle, LibrarySymbol.toU8String().c_str());
#endif
        }
    };
}
