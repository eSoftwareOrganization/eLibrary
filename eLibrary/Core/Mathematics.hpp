#pragma once

#include <Core/Number.hpp>

#include <numbers>

namespace eLibrary {
    class Mathematics final {
    public:
        Mathematics() = delete;

        template<class T>
        static typename std::enable_if<std::is_floating_point<T>::value, T>::type doCosine(T NumberSource) noexcept {
            T NumberResult = 1, NumberTerminate = 1;
            unsigned short NumberDigit = 2;
            while (std::abs(NumberTerminate) > 1e-12) {
                NumberTerminate *= -(NumberSource * NumberSource / (NumberDigit * (NumberDigit - 1)));
                NumberResult += NumberTerminate;
                NumberDigit += 2;
            }
            return NumberResult;
        }

        template<class T>
        static typename std::enable_if<std::is_floating_point<T>::value, T>::type doEvolution(T NumberBase, T NumberPower) noexcept {
            return doPower(NumberBase, 1.0 / NumberPower);
        }

        template<class T>
        static typename std::enable_if<std::is_floating_point<T>::value, T>::type doExponent(T NumberSource) noexcept {
            T NumberDenominator = 1, NumberNumerator = NumberSource, NumberResult = 1, NumberTerminate = NumberSource;
            unsigned short NumberDigit = 1;
            while (std::abs(NumberTerminate) > 1e-12) {
                NumberResult += NumberTerminate;
                NumberNumerator *= NumberSource;
                NumberDenominator = NumberDenominator * ++NumberDigit;
                NumberTerminate = NumberNumerator / NumberDenominator;
            }
            return NumberResult;
        }

        template<class T>
        static typename std::enable_if<std::is_floating_point<T>::value, T>::type doHyperbolicCosine(T NumberSource) noexcept {
            return (doExponent(NumberSource) + doExponent(-NumberSource)) / 2.0;
        }

        template<class T>
        static typename std::enable_if<std::is_floating_point<T>::value, T>::type doHyperbolicSine(T NumberSource) noexcept {
            return (doExponent(NumberSource) - doExponent(-NumberSource)) / 2.0;
        }

        template<class T>
        static typename std::enable_if<std::is_floating_point<T>::value, T>::type doHyperbolicTangent(T NumberSource) noexcept {
            return doHyperbolicSine(NumberSource) / doHyperbolicCosine(NumberSource);
        }

        template<class T>
        static typename std::enable_if<std::is_floating_point<T>::value, T>::type doInverseHyperbolicTangent(T NumberDegree) noexcept {
            T NumberNumerator = NumberDegree, NumberResult = 0, NumberTerminate = NumberDegree;
            unsigned short NumberDigit = 1;
            while (std::abs(NumberTerminate) > 1e-12) {
                NumberResult += NumberTerminate;
                NumberNumerator *= NumberDegree * NumberDegree;
                NumberDigit += 2;
                NumberTerminate = NumberNumerator / NumberDigit;
            }
            return NumberResult;
        }

        template<class T>
        static typename std::enable_if<std::is_floating_point<T>::value, T>::type doLogarithmE(T NumberSource) noexcept {
            return doInverseHyperbolicTangent((NumberSource - 1) / (NumberSource + 1)) * 2;
        }

        template<class T>
        static typename std::enable_if<std::is_floating_point<T>::value, T>::type doPower(T NumberBase, T NumberExponent) noexcept {
            return doExponent(doLogarithmE(NumberBase) * NumberExponent);
        }

        template<class T>
        static typename std::enable_if<std::is_floating_point<T>::value, T>::type doSine(T NumberSource) noexcept {
            NumberSource -= (int)std::abs(NumberSource / (std::numbers::pi * 2)) * 2 * std::numbers::pi;
            if (NumberSource == 0) return 0;
            T NumberDenominator = 1, NumberNumerator = NumberSource, NumberResult = 0, NumberSignature = 1, NumberTerminate = NumberSource;
            unsigned short NumberDigit = 1;
            while (std::abs(NumberTerminate) >= 1e-12) {
                NumberResult += NumberTerminate;
                ++NumberDigit;
                NumberSignature = -NumberSignature;
                NumberDenominator *= ((NumberDigit << 1) - 2) * ((NumberDigit << 1) - 1);
                NumberNumerator *= NumberSource * NumberSource;
                NumberTerminate = NumberSignature * (NumberNumerator / NumberDenominator);
            }
            return NumberResult;
        }

        template<class T>
        static typename std::enable_if<std::is_floating_point<T>::value, T>::type doTangent(T NumberSource) noexcept {
            return doSine(NumberSource) / doCosine(NumberSource);
        }

        static Integer getGreatestCommonFactor(const Integer &Number1, const Integer &Number2) noexcept {
            if (Number2.getAbsolute().doCompare(Integer(0)) == 0) {
                return Number1;
            }
            return getGreatestCommonFactor(Number2, Number1.doModulo(Number2));
        }

        template<typename T>
        static typename std::enable_if<std::is_integral<T>::value, T>::type getNearestPower2Lower(T NumberSource) noexcept {
            NumberSource |= NumberSource >> 1;
            NumberSource |= NumberSource >> 2;
            NumberSource |= NumberSource >> 4;
            NumberSource |= NumberSource >> 8;
            return (NumberSource + 1) >> 1;
        }

        template<typename T>
        static typename std::enable_if<std::is_integral<T>::value, T>::type getNearestPower2Upper(T NumberSource) noexcept {
            NumberSource -= 1;
            NumberSource |= NumberSource >> 1;
            NumberSource |= NumberSource >> 2;
            NumberSource |= NumberSource >> 4;
            NumberSource |= NumberSource >> 8;
            NumberSource |= NumberSource >> 16;
            return (NumberSource < 0) ? 1 : NumberSource + 1;
        }

        template<typename T>
        static T toDegrees(T NumberSource) noexcept {
            return 180.0 / std::numbers::pi * NumberSource;
        }

        template<typename T>
        static T toRadians(T NumberSource) noexcept {
            return std::numbers::pi / 180.0 * NumberSource;
        }
    };
}
