#pragma once

#ifndef eLibraryHeaderIOStream
#define eLibraryHeaderIOStream

#if eLibraryFeature(IO)

#include <IO/Buffer.hpp>
#include <IO/IOException.hpp>

namespace eLibrary::IO {
    class InputStream : public Object, public NonCopyable {
    public:
        virtual void doClose() {}

        virtual int doRead() = 0;

        virtual void doRead(ByteBuffer &StreamBuffer) {
            if (!isAvailable()) throw IOException(u"InputStream::doRead(ByteBuffer&) isAvailable"_S);
            int StreamCharacter = doRead();
            if (StreamCharacter == -1) return;
            StreamBuffer.setValue((uint8_t) StreamCharacter);
            for (uintmax_t StreamBufferIndex = 0;StreamBufferIndex < StreamBuffer.getRemaining();++StreamBufferIndex) {
                StreamCharacter = doRead();
                if (StreamCharacter == -1) return;
                StreamBuffer.setValue((uint8_t) StreamCharacter);
            }
        }

        virtual bool isAvailable() const noexcept {
            return false;
        }
    };

    class OutputStream : public Object, public NonCopyable {
    public:
        virtual void doClose() {}

        virtual void doFlush() {}

        virtual void doWrite(uint8_t) = 0;

        virtual void doWrite(const ByteBuffer &StreamBuffer) {
            if (!isAvailable()) throw IOException(u"OutputStream::doWrite(const ByteBuffer&) isAvailable"_S);
            for (uintmax_t StreamIndex = 0;StreamIndex < StreamBuffer.getRemaining();++StreamIndex)
                doWrite(StreamBuffer.getValue());
        }

        virtual bool isAvailable() const noexcept {
            return false;
        }
    };
}
#endif

#endif
