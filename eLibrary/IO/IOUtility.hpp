#pragma once

#if eLibraryFeature(IO)

#include <IO/Exception.hpp>
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>

namespace eLibrary::IO {
    class File : public Object {
    private:
        String FilePath;

        doDisableCopyAssignConstruct(File)
    public:
        explicit File(const String &FilePathSource) noexcept : FilePath(FilePathSource) {}

        void doRemove() const {
            if (::remove(FilePath.toU8String().c_str()))
                throw IOException(String(u"File::doRemove() remove"));
        }

        auto getAccessTime() const {
            struct _stat FileStatistic{};
            if (_stat(FilePath.toU8String().c_str(), &FileStatistic))
                throw IOException(String(u"File::getAccessTime() _stat"));
            return FileStatistic.st_atime;
        }

        auto getFileSize() const {
            struct _stat FileStatistic{};
            if (_stat(FilePath.toU8String().c_str(), &FileStatistic))
                throw IOException(String(u"File::getFileSize() _stat"));
            return FileStatistic.st_size;
        }

        auto getModificationTime() const {
            struct _stat FileStatistic{};
            if (_stat(FilePath.toU8String().c_str(), &FileStatistic))
                throw IOException(String(u"File::getModificationTime() _stat"));
            return FileStatistic.st_mtime;
        }

        bool isDirectory() const {
            struct _stat FileStatistic{};
            if (_stat(FilePath.toU8String().c_str(), &FileStatistic))
                throw IOException(String(u"File::isDirectory() _stat"));
            return FileStatistic.st_mode & S_IFDIR;
        }

        bool isExists() const noexcept {
            return _access(FilePath.toU8String().c_str(), 0) != -1;
        }

        bool isFile() const {
            struct _stat FileStatistic{};
            if (_stat(FilePath.toU8String().c_str(), &FileStatistic))
                throw IOException(String(u"File::isFile() _stat"));
            return FileStatistic.st_mode & S_IFREG;
        }

        String toString() const noexcept override {
            return String(u"File[FilePath=").doConcat(FilePath).doConcat(u']');
        }
    };

    class FileDescriptor : public Object {
    protected:
        int DescriptorHandle = -1;

        doDisableCopyAssignConstruct(FileDescriptor)
    public:
        doEnableMoveAssignConstruct(FileDescriptor)

        constexpr FileDescriptor() noexcept = default;

        ~FileDescriptor() noexcept {
            if (isAvailable()) doClose();
        }

        void doAssign(FileDescriptor &&DescriptorSource) noexcept {
            if (Objects::getAddress(DescriptorSource) == this) return;
            if (isAvailable()) doClose();
            DescriptorHandle = DescriptorSource.DescriptorHandle;
            DescriptorSource.DescriptorHandle = -1;
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

        String toString() const noexcept override {
            return String(u"FileDescriptor[DescriptorHandle=").doConcat(String::valueOf(DescriptorHandle)).doConcat(u']');
        }
    };

    enum class FileOption : int {
        OptionAppend = O_APPEND, OptionBinary = O_BINARY, OptionCreate = O_CREAT, OptionText = O_TEXT, OptionTruncate = O_TRUNC
    };
}
#endif
