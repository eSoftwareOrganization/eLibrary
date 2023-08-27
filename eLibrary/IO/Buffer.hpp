#pragma once

#if eLibraryFeature(IO)

#include <IO/Exception.hpp>

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
                throw IOException(String(u"Buffer::setBufferLimit(uintmax_t) BufferLimitSource"));
            BufferLimit = BufferLimitSource;
        }

        void setBufferPosition(uintmax_t BufferPositionSource) const {
            if (BufferPositionSource > BufferLimit)
                throw IOException(String(u"Buffer::setBufferPosition(uintmax_t) BufferPositionSource"));
            BufferPosition = BufferPositionSource;
        }
    };

    class ByteBuffer : public Buffer {
    protected:
        uint8_t *BufferContainer = nullptr;

        constexpr ByteBuffer(uint8_t *BufferContainerSource, uintmax_t BufferCapacitySource) noexcept : BufferContainer(BufferContainerSource) {
            BufferCapacity = BufferLimit = BufferCapacitySource;
        }

        friend class FileOutputStream;
    public:
        doEnableCopyAssignConstruct(ByteBuffer)

        constexpr ByteBuffer() noexcept = default;

        ~ByteBuffer() noexcept {
            BufferCapacity = 0;
            BufferLimit = 0;
            BufferPosition = 0;
            delete[] BufferContainer;
            BufferContainer = nullptr;
        }

        static ByteBuffer doAllocate(uintmax_t BufferCapacitySource) noexcept {
            return {MemoryAllocator::newArray<uint8_t>(BufferCapacitySource), BufferCapacitySource};
        }

        void doAssign(const ByteBuffer &BufferSource) noexcept {
            if (Objects::getAddress(BufferSource) == this) return;
            BufferPosition = BufferSource.BufferPosition;
            delete[] BufferContainer;
            BufferContainer = MemoryAllocator::newArray<uint8_t>(BufferCapacity = BufferSource.BufferCapacity);
            Arrays::doCopy(BufferSource.BufferContainer, BufferLimit = BufferSource.BufferLimit, BufferContainer);
        }

        void doCompact() noexcept {
            Arrays::doCopyBackward(BufferContainer + BufferPosition, BufferContainer + BufferPosition + getRemaining(), BufferContainer);
            BufferPosition = getRemaining();
            BufferLimit = BufferCapacity;
        }

        intmax_t doCompare(const ByteBuffer &BufferSource) const noexcept {
            if (BufferCapacity != BufferSource.BufferCapacity) return Numbers::doCompare(BufferCapacity, BufferSource.BufferCapacity);
            return ::memcmp(BufferContainer, BufferSource.BufferContainer, sizeof(uint8_t) * BufferCapacity);
        }

        uint8_t *getBufferContainer() const noexcept {
            return BufferContainer;
        }

        uint8_t getValue() const {
            if (BufferPosition >= BufferLimit) throw IOException(String(u"ByteBuffer::setValue(uint8_t) BufferPosition"));
            return BufferContainer[BufferPosition++];
        }

        uint8_t getValue(uintmax_t ValueIndex) const {
            if (ValueIndex >= BufferLimit) throw IOException(String(u"ByteBuffer::getValue(uintmax_t) ValueIndex"));
            return BufferContainer[ValueIndex];
        }

        void setValue(uint8_t ValueSource) {
            if (BufferPosition >= BufferLimit) throw IOException(String(u"ByteBuffer::setValue(uint8_t) BufferPosition"));
            BufferContainer[BufferPosition++] = ValueSource;
        }

        void setValue(uint8_t ValueSource, uintmax_t ValueIndex) {
            if (ValueIndex >= BufferLimit) throw IOException(String(u"ByteBuffer::setValue(uint8_t, uintmax_t)"));
            BufferContainer[ValueIndex] = ValueSource;
        }
    };
}
#endif
