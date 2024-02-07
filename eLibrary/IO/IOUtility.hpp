#pragma once

#ifndef eLibraryHeaderIOUtility
#define eLibraryHeaderIOUtility

#if eLibraryFeature(IO)

#include <IO/IOException.hpp>
#include <fcntl.h>
#include <sys/stat.h>
#if __has_include(<io.h>)
#include <io.h>
#endif
#if __has_include(<unistd.h>)
#include <unistd.h>
#endif

namespace eLibrary::IO {
    class File : public Object, public NonCopyable {
    private:
        String FilePath;
    public:
        explicit File(const String &FilePathSource) noexcept : FilePath(FilePathSource) {}

        void doRemove() const {
            if (::remove(FilePath.toU8String().c_str()))
                throw IOException(u"File::doRemove() remove"_S);
        }

        auto getAccessTime() const {
            struct stat FileStatistic{};
            if (::stat(FilePath.toU8String().c_str(), &FileStatistic))
                throw IOException(u"File::getAccessTime() stat"_S);
            return FileStatistic.st_atime;
        }

        auto getFileSize() const {
            struct stat FileStatistic{};
            if (::stat(FilePath.toU8String().c_str(), &FileStatistic))
                throw IOException(u"File::getFileSize() stat"_S);
            return FileStatistic.st_size;
        }

        auto getModificationTime() const {
            struct stat FileStatistic{};
            if (::stat(FilePath.toU8String().c_str(), &FileStatistic))
                throw IOException(u"File::getModificationTime() stat"_S);
            return FileStatistic.st_mtime;
        }

        bool isDirectory() const {
            struct stat FileStatistic{};
            if (::stat(FilePath.toU8String().c_str(), &FileStatistic))
                throw IOException(u"File::isDirectory() stat"_S);
            return FileStatistic.st_mode & S_IFDIR;
        }

        bool isExists() const noexcept {
            return access(FilePath.toU8String().c_str(), 0) != -1;
        }

        bool isFile() const {
            struct stat FileStatistic{};
            if (::stat(FilePath.toU8String().c_str(), &FileStatistic))
                throw IOException(u"File::isFile() stat"_S);
            return FileStatistic.st_mode & S_IFREG;
        }

        String toString() const noexcept override {
            return u"File[FilePath="_S.doConcat(FilePath).doConcat(u']');
        }
    };

    class FileDescriptor : public Object, public NonCopyable {
    protected:
        int DescriptorHandle = -1;
    public:
        doEnableMoveAssignConstruct(FileDescriptor)

        constexpr FileDescriptor() noexcept = default;

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
            if (!isAvailable()) throw IOException(u"FileDescriptor::doClose() isAvailable"_S);
            ::close(DescriptorHandle);
            DescriptorHandle = -1;
        }

        bool isAvailable() const noexcept {
            return DescriptorHandle != -1;
        }

        explicit operator int() const noexcept {
            return DescriptorHandle;
        }

        String toString() const noexcept override {
            return u"FileDescriptor[DescriptorHandle="_S.doConcat(String::valueOf(DescriptorHandle)).doConcat(u']');
        }
    };

    enum class FileOption : int {
        OptionAppend = O_APPEND, OptionCreate = O_CREAT, OptionTruncate = O_TRUNC,
#ifdef O_BINARY
        OptionBinary = O_BINARY
#else
        OptionBinary = 0
#endif
    };
}
#endif

#endif
