#pragma once

#if eLibraryFeature(IO)

#include <IO/Exception.hpp>

#include <fcntl.h>
#include <io.h>
#include <unistd.h>

namespace eLibrary::IO {
    class FileDescriptor final : public Object {
    private:
        FILE *DescriptorHandle;

        doDisableCopyAssignConstruct(FileDescriptor)
    public:
        constexpr FileDescriptor() noexcept : DescriptorHandle(nullptr) {}

        ~FileDescriptor() noexcept {
            if (isAvailable()) doClose();
        }

        void doAssign(FILE *DescriptorHandleSource) noexcept {
            if (isAvailable()) doClose();
            DescriptorHandle = DescriptorHandleSource;
        }

        void doClose() {
            if (!isAvailable()) throw IOException(String(u"FileDescriptor::doClose() isAvailable"));
            fclose(DescriptorHandle);
            DescriptorHandle = nullptr;
        }

        void doOpen(const String &DescriptorPath, const String &DescriptorMode) {
            if (isAvailable()) throw IOException(String(u"FileDescriptor::doOpen(const String&, const String&) isAvailable"));
            DescriptorHandle = fopen(DescriptorPath.toU8String().c_str(), DescriptorMode.toU8String().c_str());
            if (!isAvailable()) throw IOException(String(u"FileDescriptor::doOpen(const String&, const String&) isAvailable"));
        }

        bool isAvailable() const noexcept {
            return DescriptorHandle;
        }

        explicit operator FILE*() const noexcept {
            return DescriptorHandle;
        }
    };
}
#endif
