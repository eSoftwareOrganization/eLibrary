#pragma once

#include <Core/Object.hpp>

namespace eLibrary {
    typedef unsigned char byte;

    class InputStream : public Object {
    public:
        virtual void doClose() {}

        virtual int doRead() = 0;

        virtual int doRead(byte StreamBuffer[], int StreamOffset, int StreamSize) {
            if (StreamSize == 0) return 0;
            int StreamResult = doRead();
            if (StreamResult == -1) return -1;
            StreamBuffer[StreamOffset] = (byte) StreamResult;
            int StreamEpoch = 1;
            try {
                for (; StreamEpoch < StreamSize; ++StreamEpoch) {
                    StreamResult = doRead();
                    if (StreamResult == -1) break;
                    StreamBuffer[StreamOffset + StreamEpoch] = (byte) StreamResult;
                }
            } catch (IOException &ExceptionIgnored) {}
            return StreamEpoch;
        }

        uintmax_t doSkip(uintmax_t SkipSize) {
            byte *StreamBuffer = new byte[SkipSize];
            uintmax_t StreamRemain = SkipSize;
            while (StreamRemain) {
                int StreamResult = doRead(StreamBuffer, 0, (int) std::min(SkipSize, StreamRemain));
                if (StreamResult < 0) break;
                StreamRemain -= std::min(StreamRemain, (uintmax_t) StreamResult);
            }
            delete[] StreamBuffer;
            return SkipSize - StreamRemain;
        }

        virtual bool isAvailable() const noexcept {
            return false;
        }
    };

    class OutputStream : public Object {
    public:
        virtual void doClose() {}

        virtual void doFlush() {}

        virtual void doWrite(int StreamSource) = 0;

        virtual void doWrite(byte StreamBuffer[], int StreamOffset, int StreamSize) {
            for (int StreamCurrent = 0;StreamCurrent < StreamSize;++StreamCurrent)
                doWrite(StreamBuffer[StreamOffset + StreamCurrent]);
        }
    };
}
