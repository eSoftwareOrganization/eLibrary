#pragma once

#ifndef eLibraryHeaderPlatformWindows
#define eLibraryHeaderPlatformWindows

#include <Core/Exception.hpp>

#include <handleapi.h>

namespace eLibrary::Platform::Windows {
    using namespace eLibrary::Core;

    class NtHandle final {
    private:
        HANDLE HandleObject = INVALID_HANDLE_VALUE;
    public:
        doEnableCopyAssignConstruct(NtHandle)
        doEnableMoveAssignConstruct(NtHandle)
        doEnableValueAssignParameterConstruct(NtHandle, HANDLE)

        constexpr NtHandle() noexcept = default;

        ~NtHandle() noexcept {
            if (HandleObject != INVALID_HANDLE_VALUE) ::CloseHandle(HandleObject);
        }

        void doAssign(HANDLE HandleSource) noexcept {
            if (HandleObject != INVALID_HANDLE_VALUE) ::CloseHandle(HandleObject);
            HandleObject = HandleSource;
        }

        void doAssign(const NtHandle &HandleSource) noexcept {
            if (Objects::getAddress(HandleSource) == this) return;
            if (HandleObject != INVALID_HANDLE_VALUE) ::CloseHandle(HandleObject);
            ::DuplicateHandle(GetCurrentProcess(), HandleSource.HandleObject, GetCurrentProcess(), &HandleObject, 0, FALSE, DUPLICATE_SAME_ACCESS);
        }

        void doAssign(NtHandle &&HandleSource) noexcept {
            if (Objects::getAddress(HandleSource) == this) return;
            HandleObject = HandleSource.HandleObject;
            HandleSource.HandleObject = INVALID_HANDLE_VALUE;
        }

        void doClose() {
            if (HandleObject == INVALID_HANDLE_VALUE) doThrowChecked(Exception(u"NtHandle::doClose() HandleObject"_S));
            ::CloseHandle(HandleObject);
            HandleObject = INVALID_HANDLE_VALUE;
        }

        bool isAvailable() const noexcept {
            return HandleObject != INVALID_HANDLE_VALUE;
        }

        operator bool() const noexcept {
            return isAvailable();
        }

        explicit operator HANDLE() const noexcept {
            return HandleObject;
        }
    };
}

#endif
