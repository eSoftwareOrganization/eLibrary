#pragma once

#if eLibraryFeature(IO)

#include <IO/File.hpp>
#include <IO/Stream.hpp>

namespace eLibrary::IO {
    /**
     * The FileInputStream class provides support for reading files
     */
    class FileInputStream final : public InputStream {
    private:
        FileDescriptor StreamDescriptor;

        doDisableCopyAssignConstruct(FileInputStream)
    public:
        constexpr FileInputStream() noexcept = default;

        explicit FileInputStream(const String &StreamPath) {
            doOpen(StreamPath);
        }

        void doClose() override {
            StreamDescriptor.doClose();
        }

        void doOpen(const String &StreamPath) {
            StreamDescriptor.doOpen(StreamPath, String(u"rb"));
        }

        int doRead() override {
            if (!isAvailable()) throw IOException(String(u"FileInputStream::doRead() isAvailable"));
            return fgetc((FILE*) StreamDescriptor);
        }

        uint32_t doRead(uint8_t *StreamBuffer, uint32_t StreamBufferOffset, uint32_t StreamBufferSize) override {
            if (!isAvailable()) throw IOException(String(u"FileInputStream::doRead(uint8_t*, uint32_t, uint32_t) isAvailable"));
            return ::fread(StreamBuffer + StreamBufferOffset, 1, StreamBufferSize, (FILE*) StreamDescriptor);
        }

        void doSeek(off64_t FileOffset, int FileOrigin) const {
            if (!isAvailable()) throw IOException(String(u"FileInputStream::doSeek(off64_t, int) isAvailable"));
            ::fseeko64((FILE*) StreamDescriptor, FileOffset, FileOrigin);
        }

        auto getFileLength() const {
            if (!isAvailable()) throw IOException(String(u"FileInputStream::getFileLength() isAvailable"));
            return ::filelength(fileno((FILE*) StreamDescriptor));
        }

        auto getFilePosition() const {
            if (!isAvailable()) throw IOException(String(u"FileInputStream::getFilePosition() isAvailable"));
            return ::ftello64((FILE*) StreamDescriptor);
        }

        bool isAvailable() const noexcept override {
            return StreamDescriptor.isAvailable();
        }
    };

    /**
     * The FileOutputStream class provides support for writing files
     */
    class FileOutputStream final : public OutputStream {
    private:
        FileDescriptor StreamDescriptor;

        doDisableCopyAssignConstruct(FileOutputStream)
    public:
        constexpr FileOutputStream() noexcept = default;

        FileOutputStream(const String &StreamPath, bool FileAppend = false) {
            doOpen(StreamPath, FileAppend);
        }

        void doClose() override {
            StreamDescriptor.doClose();
        }

        void doFlush() override {
            if (!isAvailable()) throw IOException(String(u"FileOutputStream::doFlush() isAvailable"));
            ::fflush((FILE*) StreamDescriptor);
        }

        void doOpen(const String &StreamPath, bool FileAppend = false) {
            StreamDescriptor.doOpen(StreamPath, FileAppend ? String(u"ab") : String(u"wb"));
        }

        void doSeek(off64_t FileOffset, int FileOrigin) {
            if (!isAvailable()) throw IOException(String(u"FileOutputStream::doSeek(off64_t, int) isAvailable"));
            ::fseeko64((FILE*) StreamDescriptor, FileOffset, FileOrigin);
        }

        void doTruncate(off64_t FileOffset) {
            if (!isAvailable()) throw IOException(String(u"FileOutputStream::doTruncate(off64_t) isAvailable"));
            ::ftruncate64(fileno((FILE*) StreamDescriptor), FileOffset);
        }

        void doWrite(int FileCharacter) override {
            if (!isAvailable()) throw IOException(String(u"FileOutputStream::doFlush() isAvailable"));
            ::fputc(FileCharacter, (FILE*) StreamDescriptor);
        }

        void doWrite(uint8_t *FileBuffer, uint32_t FileBufferOffset, uint32_t FileBufferSize) override {
            if (!isAvailable()) throw IOException(String(u"FileOutputStream::doWrite(uint8_t*, int, int) isAvailable"));
            ::fwrite(FileBuffer + FileBufferOffset, 1, FileBufferSize, (FILE*) StreamDescriptor);
        }

        auto getFilePosition() const {
            if (!isAvailable()) throw IOException(String(u"FileOutputStream::getFilePosition() isAvailable"));
            return ::ftello64((FILE*) StreamDescriptor);
        }

        bool isAvailable() const noexcept override {
            return StreamDescriptor.isAvailable();
        }
    };
}
#endif
