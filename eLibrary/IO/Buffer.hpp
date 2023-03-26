#pragma once

#include <IO/Exception.hpp>

using namespace eLibrary::Core;

namespace eLibrary::IO {
    class Buffer : public Object {
    protected:
        unsigned BufferCapacity;
        unsigned BufferLimit;
        intmax_t BufferMark;
        unsigned BufferPosition;
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

        unsigned getBufferCapacity() const noexcept {
            return BufferCapacity;
        }

        unsigned getBufferLimit() const noexcept {
            return BufferLimit;
        }

        intmax_t getBufferMark() const noexcept {
            return BufferMark;
        }

        unsigned getBufferPosition() const noexcept {
            return BufferPosition;
        }

        unsigned getRemaining() const noexcept {
            return hasRemaining() ? BufferLimit - BufferPosition : 0;
        }

        bool hasRemaining() const noexcept {
            return BufferPosition < BufferLimit;
        }

        void setBufferLimit(unsigned BufferLimitSource) {
            if (BufferLimitSource > BufferCapacity)
                throw IOException(String(u"Buffer::setBufferLimit(unsigned) BufferLimitSource"));
            BufferLimit = BufferLimitSource;
        }

        void setBufferPosition(unsigned BufferPositionSource) {
            if (BufferPositionSource > BufferLimit)
                throw IOException(String(u"Buffer::setBufferPosition(unsigned) BufferPositionSource"));
            BufferPosition = BufferPositionSource;
        }
    };
}
