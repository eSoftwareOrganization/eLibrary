#pragma once

#include <Core/Constant.hpp>

namespace eLibrary::Core {
    class String;

    class Object {
    public:
        virtual ~Object() noexcept = default;

        virtual intmax_t hashCode() const noexcept {
            return (intmax_t) this;
        }

        virtual String toString() const noexcept;
    };
}
