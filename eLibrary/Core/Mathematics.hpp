#pragma once

#include <Core/Number.hpp>

namespace eLibrary {
    class Mathematics final {
    public:
        Mathematics() = delete;

        static Integer getGreatestCommonFactor(const Integer &Number1, const Integer &Number2) noexcept {
            if (Number2.getAbsolute().doCompare(Integer(0)) == 0) {
                return Number1;
            }
            return getGreatestCommonFactor(Number2, Number1.doModulo(Number2));
        }
    };
}
