#pragma once

#include <Core/Exception.hpp>

namespace eLibrary {
    class Buffer : public Object {
    protected:
        int BufferCapacity;
        int BufferLimit;
        int BufferMark = -1;
        int BufferPosition = 0;
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
            if (BufferMark < 0) throw Exception(String(u"Buffer::doReset() BufferMark"));
            BufferPosition = BufferMark;
        }

        void doRewind() {
            BufferMark = -1;
            BufferPosition = 0;
        }

        int getBufferCapacity() const noexcept {
            return BufferCapacity;
        }

        int getBufferLimit() const noexcept {
            return BufferLimit;
        }

        int getBufferMark() const noexcept {
            return BufferMark;
        }

        int getBufferPosition() const noexcept {
            return BufferPosition;
        }

        int getRemaining() const noexcept {
            return hasRemaining() ? BufferLimit - BufferPosition : 0;
        }

        bool hasRemaining() const noexcept {
            return BufferPosition < BufferLimit;
        }

        void setBufferLimit(int BufferLimitSource) {
            if (BufferLimitSource > BufferCapacity || BufferLimitSource < 0)
                throw Exception(String(u"Buffer::setBufferLimit(int) BufferLimitSource"));
            BufferLimit = BufferLimitSource;
        }

        void setBufferPosition(int BufferPositionSource) {
            if (BufferPositionSource > BufferLimit || BufferPositionSource < 0)
                throw Exception(String(u"Buffer::setBufferPosition(int) BufferPositionSource"));
            BufferPosition = BufferPositionSource;
        }
    };
}
