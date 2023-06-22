#pragma once

#include <IO/Exception.hpp>

namespace eLibrary::IO {
    class InputStream : public Object {
    public:
        virtual void doClose() {}

        virtual int doRead() = 0;

        virtual int doRead(byte*, int, int) = 0;

        virtual bool isAvailable() const noexcept {
            return false;
        }
    };

    class OutputStream : public Object {
    public:
        virtual void doClose() {}

        virtual void doFlush() {}

        virtual void doWrite(int) = 0;

        virtual void doWrite(byte*, int, int) = 0;
    };
}
