#pragma once

#include <IO/Exception.hpp>
#include <IO/Stream.hpp>

#include <cstdio>
#include <unistd.h>

namespace eLibrary::IO {
    /**
     * The FileInputStream class provides support for operating files
     */
    class FileInputStream final : public InputStream {
    private:
        FILE *FileOperator;
    public:
        explicit FileInputStream(const String &FilePath) {
            FileOperator = fopen64(FilePath.toU8String().c_str(), "rb");
            if (!FileOperator) throw IOException(String(u"FileInputStream::FileInputStream() fopen"));
        }

        ~FileInputStream() noexcept {
            doClose();
        }

        void doClose() noexcept override {
            if (FileOperator) {
                fclose(FileOperator);
                FileOperator = nullptr;
            }
        }

        int doRead() override {
            if (!FileOperator) throw IOException(String(u"FileInputStream::doRead() FileOperator"));
            return feof(FileOperator) ? -1 : fgetc(FileOperator);
        }

        int doRead(byte *FileBuffer, int FileBufferOffset, int FileBufferSize) override {
            if (!FileOperator) throw IOException(String(u"FileInputStream::doRead(byte*, int, int) FileOperator"));
            fread(FileBuffer + FileBufferOffset, FileBufferSize, 1, FileOperator);
        }

        void doSeek(off64_t FileOffset, int FileOrigin) {
            if (!FileOperator) throw IOException(String(u"FileInputStream::doSeek(off64_t, int) FileOperator"));
            fseeko64(FileOperator, FileOffset, FileOrigin);
        }

        bool isAvailable() const noexcept override {
            return FileOperator;
        }

        off64_t getFileLength() const {
            if (!FileOperator) throw IOException(String(u"FileInputStream::getFileLength() FileOperator"));
            off64_t FilePosition = ftello64(FileOperator);
            fseeko64(FileOperator, 0, SEEK_END);
            off64_t FileLength = ftello64(FileOperator);
            fseeko64(FileOperator, FilePosition, SEEK_SET);
            return FileLength;
        }

        off64_t getFilePosition() const {
            if (!FileOperator) throw IOException(String(u"FileInputStream::getFilePosition() FileOperator"));
            return ftello64(FileOperator);
        }

        FileInputStream &operator=(const FileInputStream&) noexcept = delete;
    };

    /**
     * The FileOutputStream class provides support for operating files
     */
    class FileOutputStream final : public OutputStream {
    private:
        FILE *FileOperator;
    public:
        FileOutputStream(const String &FilePath, bool FileAppend) {
            FileOperator = fopen64(FilePath.toU8String().c_str(), FileAppend ? "ab" : "wb");
            if (!FileOperator) throw IOException(String(u"FileOutputStream::FileOutputStream() fopen"));
        }

        ~FileOutputStream() noexcept {
            doClose();
        }

        void doClose() noexcept override {
            if (FileOperator) {
                fclose(FileOperator);
                FileOperator = nullptr;
            }
        }

        void doFlush() override {
            if (!FileOperator) throw IOException(String(u"FileOutputStream::doFlush() FileOperator"));
            fflush(FileOperator);
        }

        void doSeek(off64_t FileOffset, int FileOrigin) {
            if (!FileOperator) throw IOException(String(u"FileOutputStream::doSeek(off64_t, int) FileOperator"));
            fseeko64(FileOperator, FileOffset, FileOrigin);
        }

        void doTruncate(off64_t FileOffset) {
            if (!FileOperator) throw IOException(String(u"FileOutputStream::doTruncate(off64_t) FileOperator"));
            ftruncate64(fileno(FileOperator), FileOffset);
        }

        void doWrite(int FileCharacter) override {
            if (!FileOperator) throw IOException(String(u"FileOutputStream::doFlush() FileOperator"));
            fputc(FileCharacter, FileOperator);
        }

        void doWrite(byte *FileBuffer, int FileBufferOffset, int FileBufferSize) override {
            if (!FileOperator) throw IOException(String(u"FileOutputStream::doWrite(byte*, int, int) FileOperator"));
            fwrite(FileBuffer + FileBufferOffset, FileBufferSize, 1, FileOperator);
        }

        off64_t getFilePosition() const {
            if (!FileOperator) throw IOException(String(u"FileOutputStream::getFilePosition() FileOperator"));
            return ftello64(FileOperator);
        }

        FileOutputStream &operator=(const FileOutputStream&) noexcept = delete;
    };
}
