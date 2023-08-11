#pragma once

#if eLibraryFeature(IO)

#include <IO/Exception.hpp>

#include <fcntl.h>

namespace eLibrary::IO {
    class FileDescriptor final : public Object {
    private:
        int DescriptorHandle;

        doDisableCopyAssignConstruct(FileDescriptor)
    public:
        constexpr FileDescriptor() noexcept : DescriptorHandle(-1) {}

        ~FileDescriptor() noexcept {
            if (isAvailable()) doClose();
        }

        void doAssign(int DescriptorHandleSource) noexcept {
            if (isAvailable()) doClose();
            DescriptorHandle = DescriptorHandleSource;
        }

        void doClose() {
            if (!isAvailable()) throw IOException(String(u"FileDescriptor::doClose() isAvailable"));
            close(DescriptorHandle);
            DescriptorHandle = -1;
        }

        bool isAvailable() const noexcept {
            return DescriptorHandle != -1;
        }

        explicit operator int() const noexcept {
            return DescriptorHandle;
        }
    };

    enum class FileOption : int {
        OptionAppend = O_APPEND, OptionBinary = O_BINARY, OptionText = O_TEXT, OptionTrucate = O_TRUNC
    };
}
#endif
