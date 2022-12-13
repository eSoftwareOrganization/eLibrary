#pragma once

#include <cstdint>

namespace eLibrary {
    typedef unsigned char byte;

    class Object {
    public:
        virtual intmax_t hashCode() const noexcept {
            return (intmax_t) this;
        }

        virtual constexpr bool isEqual(const Object &ObjectSource) const noexcept {
            return this == &ObjectSource;
        }
    };
}
