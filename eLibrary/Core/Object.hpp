#pragma once

#include <cstdint>

#include <Core/Constant.hpp>

namespace eLibrary::Core {
    class String;

    class Object {
    public:
        virtual ~Object() noexcept = default;

        virtual intmax_t hashCode() const noexcept {
            return (intmax_t) this;
        }

        virtual constexpr bool isEqual(const Object &ObjectSource) const noexcept {
            return this == &ObjectSource;
        }

        virtual String toString() const noexcept;
    };
}
