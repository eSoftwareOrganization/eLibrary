#pragma once

#include <IO/Exception.hpp>

#include <dlfcn.h>

namespace eLibrary::IO {
    /**
     * Support for loading libraries at runtime
     */
    class Library final : public Object {
    private:
        void *LibraryHandle;

        constexpr Library(void *LibraryHandleSource) noexcept : LibraryHandle(LibraryHandleSource) {}

        doDisableCopyAssignConstruct(Library)
    public:
        ~Library() noexcept {
            if (LibraryHandle) {
                dlclose(LibraryHandle);
                LibraryHandle = nullptr;
            }
        }

        static Library doOpen(const String &LibraryPath) {
            void *LibraryHandle = dlopen(LibraryPath.toU8String().c_str(), RTLD_LAZY);
            if (!LibraryHandle) throw IOException(String(u"Library::Library(const String&)"));
            return {LibraryHandle};
        }

        void *getSymbol(const String &LibrarySymbol) const {
            if (!LibraryHandle) return nullptr;
            return dlsym(LibraryHandle, LibrarySymbol.toU8String().c_str());
        }
    };
}
