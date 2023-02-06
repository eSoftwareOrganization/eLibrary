#pragma once

#include <Core/Object.hpp>

namespace eLibrary {
    class InputStream : public Object {
    public:
        virtual void doClose() {}

        virtual int doRead() = 0;

        virtual bool isAvailable() const noexcept {
            return false;
        }
    };

    class OutputStream : public Object {
    public:
        virtual void doClose() {}

        virtual void doFlush() {}

        virtual void doWrite(int) = 0;
    };
}
