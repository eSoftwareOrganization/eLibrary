#pragma once

#ifndef eLibraryHeaderIOBuffer
#define eLibraryHeaderIOBuffer

#if eLibraryFeature(IO)

#include <IO/IOException.hpp>

namespace eLibrary::IO {
    class Buffer : public Object {
    protected:
        uintmax_t BufferCapacity = 0;
        uintmax_t BufferLimit = 0;
        mutable uintmax_t BufferPosition = 0;

        constexpr Buffer() noexcept = default;
    public:
        void doClear() noexcept {
            BufferLimit = BufferCapacity;
            BufferPosition = 0;
        }

        void doFlip() noexcept {
            BufferLimit = BufferPosition;
            BufferPosition = 0;
        }

        void doRewind() {
            BufferPosition = 0;
        }

        uintmax_t getBufferCapacity() const noexcept {
            return BufferCapacity;
        }

        uintmax_t getBufferLimit() const noexcept {
            return BufferLimit;
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
                doThrowChecked(IOException, u"Buffer::setBufferLimit(uintmax_t) BufferLimitSource"_S);
            BufferLimit = BufferLimitSource;
        }

        void setBufferPosition(uintmax_t BufferPositionSource) const {
            if (BufferPositionSource > BufferLimit)
                doThrowChecked(IOException, u"Buffer::setBufferPosition(uintmax_t) BufferPositionSource"_S);
            BufferPosition = BufferPositionSource;
        }
    };

    class ByteBuffer : public Buffer {
    protected:
        Array<uint8_t> BufferContainer;

        friend class FileInputStream;
        friend class FileOutputStream;
    public:
        constexpr ByteBuffer() noexcept = default;

        ByteBuffer(const Array<uint8_t> &BufferContainerSource) noexcept : BufferContainer(BufferContainerSource) {
            BufferCapacity = BufferLimit = BufferContainerSource.getElementSize();
        }

        static ByteBuffer doAllocate(uintmax_t BufferCapacitySource) noexcept {
            return {Array<uint8_t>(BufferCapacitySource)};
        }

        void doCompact() noexcept {
            Collections::doCopyBackward(BufferContainer.begin() + BufferPosition, BufferContainer.begin() + BufferPosition + getRemaining(), BufferContainer.begin());
            BufferPosition = getRemaining();
            BufferLimit = BufferCapacity;
        }

        intmax_t doCompare(const ByteBuffer &BufferSource) const noexcept {
            return Collections::doCompare(BufferContainer.begin() + BufferPosition, BufferContainer.begin() + BufferLimit, BufferSource.BufferContainer.begin() + BufferSource.BufferPosition, BufferSource.BufferContainer.begin() + BufferSource.BufferLimit);
        }

        Array<uint8_t> getBufferContainer() const noexcept {
            return BufferContainer;
        }

        uint8_t getValue() const {
            if (BufferPosition >= BufferLimit) doThrowChecked(IOException, u"ByteBuffer::setValue(uint8_t) BufferPosition"_S);
            return BufferContainer.getElement(BufferPosition++);
        }

        uint8_t getValue(uintmax_t ValueIndex) const {
            if (ValueIndex >= BufferLimit) doThrowChecked(IOException, u"ByteBuffer::getValue(uintmax_t) ValueIndex"_S);
            return BufferContainer.getElement(ValueIndex);
        }

        void setValue(uint8_t ValueSource) {
            if (BufferPosition >= BufferLimit) doThrowChecked(IOException, u"ByteBuffer::setValue(uint8_t) BufferPosition"_S);
            BufferContainer.getElement(BufferPosition++) = ValueSource;
        }

        void setValue(uint8_t ValueSource, uintmax_t ValueIndex) {
            if (ValueIndex >= BufferLimit) doThrowChecked(IOException, u"ByteBuffer::setValue(uint8_t, uintmax_t)"_S);
            BufferContainer.getElement(ValueIndex) = ValueSource;
        }
    };
}
#endif

#endif
