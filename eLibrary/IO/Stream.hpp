#pragma once

#if eLibraryFeature(IO)

#include <IO/Exception.hpp>

namespace eLibrary::IO {
    class InputStream : public Object {
    public:
        virtual void doClose() {}

        virtual int doRead() = 0;

        virtual uint32_t doRead(uint8_t *StreamBuffer, uint32_t StreamBufferOffset, uint32_t StreamBufferSize) {
            if (!isAvailable()) throw IOException(String(u"InputStream::doRead(uint8_t*, uint32_t, uint32_t) isAvailable"));
            int StreamCharacter = doRead();
            if (StreamCharacter == -1) return 0;
            StreamBuffer[StreamBufferOffset] = (uint8_t) StreamCharacter;
            for (uint32_t StreamBufferIndex = 0;StreamBufferIndex < StreamBufferSize;++StreamBufferIndex) {
                StreamCharacter = doRead();
                if (StreamCharacter == -1) return StreamBufferIndex;
                StreamBuffer[StreamBufferOffset + StreamBufferIndex] = (uint8_t) StreamCharacter;
            }
            return StreamBufferSize;
        }

        virtual bool isAvailable() const noexcept {
            return false;
        }
    };

    class OutputStream : public Object {
    public:
        virtual void doClose() {}

        virtual void doFlush() {}

        virtual void doWrite(uint8_t) = 0;

        virtual void doWrite(uint8_t *StreamBuffer, uint32_t StreamOffset, uint32_t StreamSize) {
            if (!isAvailable()) throw IOException(String(u"OutputStream::doWrite(uint8_t*, uint32_t, uint32_t) isAvailable"));
            for (uint32_t StreamIndex = 0;StreamIndex < StreamSize;++StreamIndex)
                doWrite(StreamBuffer[StreamOffset + StreamIndex]);
        }

        virtual bool isAvailable() const noexcept {
            return false;
        }
    };
}
#endif
