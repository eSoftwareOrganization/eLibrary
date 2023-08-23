#pragma once

#if eLibraryFeature(IO)

#include <IO/Exception.hpp>
#include <fcntl.h>
#include <io.h>

namespace eLibrary::IO {
    class FileDescriptor : public Object {
    protected:
        int DescriptorHandle;

        doDisableCopyAssignConstruct(FileDescriptor)
    public:
        constexpr FileDescriptor() noexcept : DescriptorHandle(-1) {}

        constexpr FileDescriptor(FileDescriptor &&DescriptorSource) noexcept {
            DescriptorHandle = DescriptorSource.DescriptorHandle;
            DescriptorSource.DescriptorHandle = -1;
        }

        ~FileDescriptor() noexcept {
            if (isAvailable()) doClose();
        }

        void doAssign(int DescriptorHandleSource) noexcept {
            if (isAvailable()) doClose();
            DescriptorHandle = DescriptorHandleSource;
        }

        virtual void doClose() {
            if (!isAvailable()) throw IOException(String(u"FileDescriptor::doClose() isAvailable"));
            _close(DescriptorHandle);
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
        OptionAppend = O_APPEND, OptionBinary = O_BINARY, OptionCreate = O_CREAT, OptionText = O_TEXT, OptionTruncate = O_TRUNC
    };
}
#endif
