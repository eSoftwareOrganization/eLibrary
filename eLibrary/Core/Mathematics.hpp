#pragma once

#include <Core/Number.hpp>

namespace eLibrary {
    class Mathematics final {
    public:
        Mathematics() = delete;

        template<class T>
        static T doEvolution(T NumberBase, T NumberPower) noexcept {
            return doPower(NumberBase, 1.0 / NumberPower);
        }

        template<class T>
        static T doExponent(T NumberSource) {
            T NumberDenominator = 1, NumberNumerator = NumberSource, NumberResult = 1, NumberTerminate = NumberSource;
            unsigned short NumberDigit = 1;
            while (__builtin_fabs(NumberTerminate) > 1e-12) {
                NumberResult += NumberTerminate;
                NumberNumerator *= NumberSource;
                NumberDenominator = NumberDenominator * ++NumberDigit;
                NumberTerminate = NumberNumerator / NumberDenominator;
            }
            return NumberResult;
        }

        template<class T>
        static T doInverseHyperbolicTangent(T NumberDegree) {
            T NumberNumerator = NumberDegree, NumberResult = 0, NumberTerminate = NumberDegree;
            unsigned short NumberDigit = 1;
            while (__builtin_fabs(NumberTerminate) > 1e-12) {
                NumberResult += NumberTerminate;
                NumberNumerator *= NumberDegree * NumberDegree;
                NumberDigit += 2;
                NumberTerminate = NumberNumerator / NumberDigit;
            }
            return NumberResult;
        }

        template<class T>
        static T doLogarithmE(T NumberSource) {
            return doInverseHyperbolicTangent((NumberSource - 1) / (NumberSource + 1)) * 2;
        }

        template<class T>
        static T doPower(T NumberBase, T NumberExponent) {
            return doExponent(doLogarithmE(NumberBase) * NumberExponent);
        }

        static Integer getGreatestCommonFactor(const Integer &Number1, const Integer &Number2) noexcept {
            if (Number2.getAbsolute().doCompare(Integer(0)) == 0) {
                return Number1;
            }
            return getGreatestCommonFactor(Number2, Number1.doModulo(Number2));
        }
    };
}
