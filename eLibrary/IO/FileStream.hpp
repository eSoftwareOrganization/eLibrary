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

        static int getOpenOption(FileOption OptionCurrent) noexcept {
            return (int) OptionCurrent;
        }

        template<typename ...Os>
        static int getOpenOption(FileOption OptionCurrent, Os ...OptionList) noexcept {
            return (int) OptionCurrent | getOpenOption(OptionList...);
        }

        doDisableCopyAssignConstruct(FileInputStream)
    public:
        constexpr FileInputStream() noexcept = default;

        void doClose() override {
            StreamDescriptor.doClose();
        }

        template<typename ...Os>
        void doOpen(const String &StreamPath, Os ...StreamOptionList) {
            if (isAvailable()) throw IOException(String(u"FileInputStream::doOpen<Os...>(const String&, Os...) isAvailable"));
            int StreamOption = O_RDONLY;
            if constexpr (sizeof...(StreamOptionList) > 0) StreamOption |= getOpenOption(StreamOptionList...);
            StreamDescriptor.doAssign(_open(StreamPath.toU8String().c_str(), StreamOption));
            if (!isAvailable()) throw IOException(String(u"FileInputStream::doOpen<Os...>(const String&, Os...) isAvailable"));
        }

        int doRead() override {
            if (!isAvailable()) throw IOException(String(u"FileInputStream::doRead() isAvailable"));
            char StreamBuffer;
            ::_read((int) StreamDescriptor, &StreamBuffer, 1);
            return StreamBuffer;
        }

        void doRead(ByteBuffer &FileBuffer) override {
            if (!isAvailable()) throw IOException(String(u"FileInputStream::doRead(ByteBuffer&) isAvailable"));
            FileBuffer.setBufferPosition(FileBuffer.getBufferPosition() + ::_read((int) StreamDescriptor, FileBuffer.getBufferContainer() + FileBuffer.getBufferPosition(), FileBuffer.getRemaining()));
        }

        void doSeek(long FileOffset, int FileOrigin) const {
            if (!isAvailable()) throw IOException(String(u"FileInputStream::doSeek(long, int) isAvailable"));
            ::_lseek((int) StreamDescriptor, FileOffset, FileOrigin);
        }

        auto getFileLength() const {
            if (!isAvailable()) throw IOException(String(u"FileInputStream::getFileLength() isAvailable"));
            return ::_filelength((int) StreamDescriptor);
        }

        auto getFilePosition() const {
            if (!isAvailable()) throw IOException(String(u"FileInputStream::getFilePosition() isAvailable"));
            return ::_tell((int) StreamDescriptor);
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

        static int getOpenOption(FileOption OptionCurrent) noexcept {
            return (int) OptionCurrent;
        }

        template<typename ...Os>
        static int getOpenOption(FileOption OptionCurrent, Os ...OptionList) noexcept {
            return (int) OptionCurrent | getOpenOption(OptionList...);
        }

        doDisableCopyAssignConstruct(FileOutputStream)
    public:
        constexpr FileOutputStream() noexcept = default;

        void doClose() override {
            StreamDescriptor.doClose();
        }

        void doFlush() override {
            if (!isAvailable()) throw IOException(String(u"FileOutputStream::doFlush() isAvailable"));
        }

        template<typename ...Os>
        void doOpen(const String &StreamPath, Os ...StreamOptionList) {
            if (isAvailable()) throw IOException(String(u"FileOutputStream::doOpen<Os...>(const String&, Os...) isAvailable"));
            int StreamOption = O_WRONLY;
            if constexpr (sizeof...(StreamOptionList) > 0) StreamOption |= getOpenOption(StreamOptionList...);
            StreamDescriptor.doAssign(_open(StreamPath.toU8String().c_str(), StreamOption));
            if (!isAvailable()) throw IOException(String(u"FileOutputStream::doOpen<Os...>(const String&, Os...) isAvailable"));
        }

        void doSeek(long FileOffset, int FileOrigin) {
            if (!isAvailable()) throw IOException(String(u"FileOutputStream::doSeek(long, int) isAvailable"));
            ::_lseek((int) StreamDescriptor, FileOffset, FileOrigin);
        }

        void doWrite(uint8_t FileCharacter) override {
            if (!isAvailable()) throw IOException(String(u"FileOutputStream::doWrite(uint8_t) isAvailable"));
            ::_write((int) StreamDescriptor, &FileCharacter, 1);
        }

        void doWrite(const ByteBuffer &FileBuffer) override {
            if (!isAvailable()) throw IOException(String(u"FileOutputStream::doWrite(const ByteBuffer&) isAvailable"));
            ::_write((int) StreamDescriptor, FileBuffer.BufferContainer + FileBuffer.getBufferPosition(), FileBuffer.getRemaining());
            FileBuffer.setBufferPosition(FileBuffer.getBufferLimit());
        }

        auto getFilePosition() const {
            if (!isAvailable()) throw IOException(String(u"FileOutputStream::getFilePosition() isAvailable"));
            return ::_tell((int) StreamDescriptor);
        }

        bool isAvailable() const noexcept override {
            return StreamDescriptor.isAvailable();
        }
    };
}
#endif
