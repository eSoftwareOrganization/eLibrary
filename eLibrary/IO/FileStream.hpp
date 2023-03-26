#pragma once

#include <IO/Exception.hpp>
#include <IO/Stream.hpp>

#include <cstdio>
#include <unistd.h>

using namespace eLibrary::Core;

namespace eLibrary::IO {
    class FileInputStream final : public InputStream {
    private:
        FILE *FileOperator;
    public:
        explicit FileInputStream(const String &FilePath) {
            FileOperator = fopen(FilePath.toU8String().c_str(), "rb");
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

        void doSeek(long FileOffset, int FileOrigin) {
            if (!FileOperator) throw IOException(String(u"FileInputStream::doSeek(long, int) FileOperator"));
            fseek(FileOperator, FileOffset, FileOrigin);
        }

        bool isAvailable() const noexcept override {
            return FileOperator;
        }

        long getFileLength() const {
            if (!FileOperator) throw IOException(String(u"FileInputStream::getFileLength() FileOperator"));
            long FilePosition = ftell(FileOperator);
            fseek(FileOperator, 0, SEEK_END);
            long FileLength = ftell(FileOperator);
            fseek(FileOperator, FilePosition, SEEK_SET);
            return FileLength;
        }

        long getFilePosition() const {
            if (!FileOperator) throw IOException(String(u"FileInputStream::getFilePosition() FileOperator"));
            return ftell(FileOperator);
        }

        FileInputStream &operator=(const FileInputStream&) noexcept = delete;
    };

    class FileOutputStream final : public OutputStream {
    private:
        FILE *FileOperator;
    public:
        FileOutputStream(const String &FilePath, bool FileAppend) {
            FileOperator = fopen(FilePath.toU8String().c_str(), FileAppend ? "ab" : "wb");
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

        void doSeek(long FileOffset, int FileOrigin) {
            if (!FileOperator) throw IOException(String(u"FileOutputStream::doSeek(long, int) FileOperator"));
            fseek(FileOperator, FileOffset, FileOrigin);
        }

        void doTruncate(long FileOffset) {
            if (!FileOperator) throw IOException(String(u"FileOutputStream::doTruncate(long) FileOperator"));
            ftruncate(fileno(FileOperator), FileOffset);
        }

        void doWrite(int FileCharacter) override {
            if (!FileOperator) throw IOException(String(u"FileOutputStream::doFlush() FileOperator"));
            fputc(FileCharacter, FileOperator);
        }

        void doWrite(byte *FileBuffer, int FileBufferOffset, int FileBufferSize) override {
            if (!FileOperator) throw IOException(String(u"FileOutputStream::doWrite(byte*, int, int) FileOperator"));
            fwrite(FileBuffer + FileBufferOffset, FileBufferSize, 1, FileOperator);
        }

        long getFilePosition() const {
            if (!FileOperator) throw IOException(String(u"FileOutputStream::getFilePosition() FileOperator"));
            return ftell(FileOperator);
        }

        FileOutputStream &operator=(const FileOutputStream&) noexcept = delete;
    };
}
