#pragma once

#if eLibraryFeature(IO)

#include <IO/Exception.hpp>

namespace eLibrary::IO {
    class Buffer : public Object {
    protected:
        uintmax_t BufferCapacity;
        uintmax_t BufferLimit;
        intmax_t BufferMark;
        mutable uintmax_t BufferPosition;
    public:
        void doClear() noexcept {
            BufferLimit = BufferCapacity;
            BufferMark = -1;
            BufferPosition = 0;
        }

        void doDiscardMark() noexcept {
            BufferMark = -1;
        }

        void doFlip() noexcept {
            BufferLimit = BufferPosition;
            BufferMark = -1;
            BufferPosition = 0;
        }

        void doMark() noexcept {
            BufferMark = BufferPosition;
        }

        void doReset() {
            if (BufferMark < 0) throw IOException(String(u"Buffer::doReset() BufferMark"));
            BufferPosition = BufferMark;
        }

        void doRewind() {
            BufferMark = -1;
            BufferPosition = 0;
        }

        uintmax_t getBufferCapacity() const noexcept {
            return BufferCapacity;
        }

        uintmax_t getBufferLimit() const noexcept {
            return BufferLimit;
        }

        intmax_t getBufferMark() const noexcept {
            return BufferMark;
        }

        uintmax_t getBufferPosition() const noexcept {
            return BufferPosition;
        }

        uintmax_t getRemaining() const noexcept {
            return hasRemaining() ? BufferLimit - BufferPosition : 0;
        }

        bool hasRemaining() const noexcept {
            return BufferPosition < BufferLimit;
        }

        void setBufferLimit(uintmax_t BufferLimitSource) {
            if (BufferLimitSource > BufferCapacity)
                throw IOException(String(u"Buffer::setBufferLimit(uintmax_t) BufferLimitSource"));
            BufferLimit = BufferLimitSource;
        }

        void setBufferPosition(uintmax_t BufferPositionSource) {
            if (BufferPositionSource > BufferLimit)
                throw IOException(String(u"Buffer::setBufferPosition(uintmax_t) BufferPositionSource"));
            BufferPosition = BufferPositionSource;
        }
    };

    class ByteBuffer : public Buffer {
    protected:
        ByteBuffer(uint8_t*, uintmax_t) {}

        uint8_t *BufferContainer;
    public:
        static ByteBuffer doAllocate(uintmax_t BufferCapacitySource) {
            return {new uint8_t[BufferCapacitySource], BufferCapacitySource};
        }

        uint8_t getValue() const {
            if (BufferPosition >= BufferLimit) throw IOException(String(u"ByteBuffer::setValue(uint8_t) BufferPosition"));
            return BufferContainer[++BufferPosition];
        }

        uint8_t getValue(uintmax_t ValueIndex) const {
            if (ValueIndex >= BufferLimit) throw IOException(String(u"ByteBuffer::getValue(uintmax_t) ValueIndex"));
            return BufferContainer[ValueIndex];
        }

        void setValue(uint8_t ValueSource) {
            if (BufferPosition >= BufferLimit) throw IOException(String(u"ByteBuffer::setValue(uint8_t) BufferPosition"));
            BufferContainer[++BufferPosition] = ValueSource;
        }

        void setValue(uint8_t ValueSource, uintmax_t ValueIndex) {
            if (ValueIndex >= BufferLimit) throw IOException(String(u"ByteBuffer::setValue(uint8_t, uintmax_t)"));
            BufferContainer[ValueIndex] = ValueSource;
        }
    };
}
#endif
