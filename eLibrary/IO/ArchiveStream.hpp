#pragma once

#if eLibraryFeature(IO)

#include <IO/Stream.hpp>

extern "C" {
#include <archive.h>
#include <archive_entry.h>
}

namespace eLibrary::IO {
    class ArchiveDescriptor final : public Object {
    private:
        archive *DescriptorHandle;

        doDisableCopyAssignConstruct(ArchiveDescriptor)
    public:
        constexpr ArchiveDescriptor() noexcept : DescriptorHandle(nullptr) {}

        ~ArchiveDescriptor() noexcept {
            if (isAvailable()) doClose();
        }

        void addPassphrase(const String &DescriptorPassphrase) {
            if (!DescriptorPassphrase.isEmpty() && ::archive_read_add_passphrase(DescriptorHandle, DescriptorPassphrase.toU8String().c_str()))
                throw IOException(String(u"ArchiveDescriptor::addPassphrase() archive_read_add_passphrase"));
        }

        void doAssign(archive *DescriptorHandleSource) noexcept {
            if (isAvailable()) doClose();
            DescriptorHandle = DescriptorHandleSource;
        }

        void doClose() {
            if (!isAvailable()) throw IOException(String(u"ArchiveDescriptor::doClose() isAvailable"));
            ::archive_read_close(DescriptorHandle);
            ::archive_read_free(DescriptorHandle);
            DescriptorHandle = nullptr;
        }

        void doSupportFilter() const {
            if (::archive_read_support_filter_all(DescriptorHandle))
                throw IOException(String(u"ArchiveDescriptor::doSupportFilter() archive_read_support_filter_all"));
        }

        void doSupportFormat() const {
            if (::archive_read_support_format_all(DescriptorHandle))
                throw IOException(String(u"ArchiveDescriptor::doSupportFormat() archive_read_support_format_all"));
        }

        bool isAvailable() const noexcept {
            return DescriptorHandle != nullptr;
        }

        explicit operator archive*() const noexcept {
            return DescriptorHandle;
        }
    };

    class ArchiveInputStream final : public InputStream {
    private:
        ArchiveDescriptor StreamDescriptor;

        doDisableCopyAssignConstruct(ArchiveInputStream)
    public:
        constexpr ArchiveInputStream() noexcept = default;

        void doClose() override {
            StreamDescriptor.doClose();
        }

        void doOpen(const String &StreamPath, const String &StreamPassphrase) {
            StreamDescriptor.doAssign(archive_read_new());
            if (!StreamDescriptor.isAvailable()) throw IOException(String(u"ArchiveInputStream::doOpen(const String&, const String&) archive_read_new"));
            StreamDescriptor.doSupportFilter();
            StreamDescriptor.doSupportFormat();
            StreamDescriptor.addPassphrase(StreamPassphrase);
            if (::archive_read_open_filename((archive*) StreamDescriptor, StreamPath.toU8String().c_str(), 10240))
                throw IOException(String(u"ArchiveInputStream::doOpen(const String&, const String&) archive_read_open_filename"));
        }

        int doRead() override {
            if (!isAvailable()) throw IOException(String(u"ArchiveInputStream::doRead() isAvailable"));
            int StreamCharacter;
            ::archive_read_data((archive*) StreamDescriptor, &StreamCharacter, 1);
            return StreamCharacter;
        }

        uint32_t doRead(uint8_t *StreamBuffer, uint32_t StreamBufferOffset, uint32_t StreamBufferSize) override {
            if (!isAvailable()) throw IOException(String(u"ArchiveInputStream::doRead(uint8_t, uint32_t, uint32_t) isAvailable"));
            return (uint32_t) ::archive_read_data((archive*) StreamDescriptor, StreamBuffer + StreamBufferOffset, StreamBufferSize);
        }

        archive_entry *doReadNextHeader() const {
            archive_entry *StreamEntry = nullptr;
            if (::archive_read_next_header((archive*) StreamDescriptor, &StreamEntry))
                throw IOException(String(u"ArchiveInputStream::doReadNextHeader() archive_read_next_header"));
            return StreamEntry;
        }

        bool isAvailable() const noexcept override {
            return StreamDescriptor.isAvailable();
        }

        bool isEncrypted() const noexcept {
            return ::archive_read_has_encrypted_entries((archive*) StreamDescriptor);
        }
    };

    class ArchiveOutputStream final : public Object {};

    class ArchiveEntry final : public Object {
    private:
        archive_entry *EntryHandle;

        doDisableCopyAssignConstruct(ArchiveEntry)

        constexpr ArchiveEntry(archive_entry *EntryHandleSource) noexcept : EntryHandle(EntryHandleSource) {}
    public:
        constexpr ArchiveEntry() noexcept : EntryHandle(nullptr) {}

        ~ArchiveEntry() noexcept {
            if (isAvailable()) doClose();
        }

        static ArchiveEntry doAllocate() {
            archive_entry *EntryHandleSource = ::archive_entry_new();
            if (!EntryHandleSource) throw IOException(String(u"ArchiveEntry::doAllocate() archive_entry_new"));
            return {EntryHandleSource};
        }

        void doAssign(archive_entry *EntryHandleSource) noexcept {
            if (isAvailable()) doClear();
            EntryHandle = EntryHandleSource;
        }

        void doClear() {
            if (!isAvailable()) throw IOException(String(u"ArchiveEntry::doClear() isAvailable"));
            ::archive_entry_clear(EntryHandle);
        }

        void doClose() {
            if (!isAvailable()) throw IOException(String(u"ArchiveEntry::doClose() isAvailable"));
            ::archive_entry_free(EntryHandle);
            EntryHandle = nullptr;
        }

        String getPathName() const noexcept {
            if (!isAvailable()) return {u""};
            return {::archive_entry_pathname(EntryHandle)};
        }

        bool isAvailable() const noexcept {
            return EntryHandle != nullptr;
        }
    };
}
#endif
