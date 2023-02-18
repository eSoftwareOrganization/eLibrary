#pragma once

#include <Core/Number.hpp>

#include <concepts>
#include <numbers>

namespace eLibrary {
    class Mathematics final : public Object {
    private:
        static bool isPrimeLucas(const Integer &NumberSource) noexcept {
            Integer NumberD(5);
            for (;;) {
                Integer NumberG(getGreatestCommonFactor(NumberD.getAbsolute(), NumberSource));
                if (NumberG.doCompare(1) > 0 && NumberG.doCompare(NumberSource)) return false;
                if (!getJocabiSymbol(NumberD, NumberSource).doCompare(-1)) break;
                if (NumberD.isPositive()) NumberD = NumberD.getOpposite().doSubtraction(2);
                else NumberD = NumberD.getOpposite().doAddition(2);
            }
            if (!NumberD.doCompare(0)) return false;
            Integer NumberIterationCount(0), NumberK(NumberSource.doAddition(1)), NumberP(1), NumberQ(Integer(1).doSubtraction(NumberD).doDivision(4));
            while (NumberK.isEven()) NumberK = NumberK.doDivision(2), NumberIterationCount = NumberIterationCount.doAddition(1);
            Integer NumberBitCount(NumberK.toString(2).getCharacterSize()), NumberQk(NumberQ), NumberU(1), NumberV(NumberP);
            if (!NumberQ.doCompare(1)) {
                while (NumberBitCount.doCompare(1) > 0) {
                    NumberU = NumberU.doMultiplication(NumberV).doModulo(NumberSource);
                    NumberV = NumberV.doMultiplication(NumberV).doSubtraction(2).doModulo(NumberSource);
                    NumberBitCount = NumberBitCount.doSubtraction(1);
                    if (NumberK.doDivision(Integer(2).doPower(NumberBitCount.doSubtraction(1))).isOdd()) {
                        Integer NumberUBackup(NumberU.doMultiplication(NumberP).doAddition(NumberV));
                        NumberV = NumberV.doMultiplication(NumberP).doAddition(NumberU.doMultiplication(NumberP));
                        NumberU = NumberUBackup;
                        if (NumberU.isOdd()) NumberU = NumberU.doAddition(NumberSource);
                        if (NumberV.isOdd()) NumberV = NumberV.doAddition(NumberSource);
                        NumberU = NumberU.doDivision(2);
                        NumberV = NumberV.doDivision(2);
                    }
                }
            } else if (!NumberP.doCompare(1) && !NumberQ.doCompare(-1)) {
                while (NumberBitCount.doCompare(1) > 0) {
                    NumberU = NumberU.doMultiplication(NumberV).doModulo(NumberSource);
                    if (!NumberQk.doCompare(1)) NumberV = NumberV.doMultiplication(NumberV).doSubtraction(2).doModulo(NumberSource);
                    else NumberV = NumberV.doMultiplication(NumberV).doAddition(2).doModulo(NumberSource), NumberQk = 1;
                    NumberBitCount = NumberBitCount.doSubtraction(1);
                    if (NumberK.doDivision(Integer(2).doPower(NumberBitCount.doSubtraction(1))).isOdd()) {
                        Integer NumberUBackup(NumberU.doAddition(NumberV));
                        NumberV = NumberV.doAddition(NumberU.doMultiplication(NumberD));
                        NumberU = NumberUBackup;
                        if (NumberU.isOdd()) NumberU = NumberU.doAddition(NumberSource);
                        if (NumberV.isOdd()) NumberV = NumberV.doAddition(NumberSource);
                        NumberU = NumberU.doDivision(2);
                        NumberV = NumberV.doDivision(2);
                        NumberQk = -1;
                    }
                }
            } else while (NumberBitCount.doCompare(1) > 0) {
                    NumberU = NumberU.doMultiplication(NumberV).doModulo(NumberSource);
                    NumberV = NumberV.doMultiplication(NumberV).doSubtraction(NumberQk.doAddition(NumberQk)).doModulo(NumberSource);
                    NumberQk = NumberQk.doMultiplication(NumberQk);
                    NumberBitCount = NumberBitCount.doSubtraction(1);
                    if (NumberK.doDivision(Integer(2).doPower(NumberBitCount.doSubtraction(1))).isOdd()) {
                        Integer NumberUBackup(NumberU.doAddition(NumberV));
                        NumberV = NumberV.doAddition(NumberU.doMultiplication(NumberD));
                        NumberU = NumberUBackup;
                        if (NumberU.isOdd()) NumberU = NumberU.doAddition(NumberSource);
                        if (NumberV.isOdd()) NumberV = NumberV.doAddition(NumberSource);
                        NumberU = NumberU.doDivision(2);
                        NumberV = NumberV.doDivision(2);
                        NumberQk = NumberQk.doMultiplication(NumberQ);
                    }
                    NumberQk = NumberQk.doModulo(NumberSource);
                }
            NumberU = NumberU.doModulo(NumberSource);
            NumberV = NumberV.doModulo(NumberSource);
            if (!NumberU.doCompare(0) || !NumberV.doCompare(0)) return true;
            for (Integer NumberIteration = 1;NumberIteration.doCompare(NumberIterationCount) < 0;NumberIteration = NumberIteration.doAddition(1)) {
                NumberV = NumberV.doMultiplication(NumberV).doSubtraction(NumberQk.doAddition(NumberQk)).doModulo(NumberSource);
                if (!NumberV.doCompare(0)) return true;
                NumberQk = NumberQk.doPower(2, NumberSource);
            }
            return false;
        }

        static bool isPrimeRabinMiller(const Integer &NumberSource, const std::vector<Integer> &NumberBaseList) noexcept {
            Integer NumberIteration(1), NumberExponent(NumberSource.doSubtraction(1));
            while (NumberExponent.isEven()) NumberExponent = NumberExponent.doDivision(2), NumberIteration = NumberIteration.doAddition(1);
            for (Integer NumberBase : NumberBaseList) {
                if (NumberBase.doCompare(NumberSource) >= 0) NumberBase = NumberBase.doModulo(NumberSource);
                if (NumberBase.doCompare(2) >= 0 && !isPrimeRabinMiller(NumberSource, NumberBase, NumberExponent, NumberIteration)) return false;
            }
            return true;
        }

        static bool isPrimeRabinMiller(const Integer &NumberSource, const Integer &NumberBase, const Integer &NumberExponent, const Integer &NumberIterationCount) noexcept {
            Integer NumberPower(NumberBase.doPower(NumberExponent, NumberSource));
            if (!NumberPower.doCompare(1) || !NumberPower.doCompare(NumberSource.doSubtraction(1))) return true;
            for (Integer NumberIteration = 0;NumberIteration.doCompare(NumberIterationCount) < 0;NumberIteration = NumberIteration.doAddition(1)) {
                NumberPower = NumberPower.doPower(2, NumberSource);
                if (!NumberPower.doCompare(NumberSource.doSubtraction(1))) return true;
                if (!NumberPower.doCompare(1)) return false;
            }
            return false;
        }
    public:
        Mathematics() = delete;

        static Integer doCombinator(const Integer &NumberM, const Integer &NumberN) {
            if (NumberN.doCompare(NumberM) < 0) throw Exception(String(u"Mathematics::doCombinator(const Integer&, const Integer&) NumberM NumberN"));
            return NumberN.doFactorial().doDivision(NumberM.doFactorial().doMultiplication(NumberN.doSubtraction(NumberM).doFactorial()));
        }

        template<std::floating_point T>
        static T doCosine(T NumberSource) noexcept {
            T NumberResult = 1, NumberTerminate = 1;
            unsigned short NumberDigit = 2;
            while (std::abs(NumberTerminate) > 1e-12) {
                NumberTerminate *= -(NumberSource * NumberSource / (NumberDigit * (NumberDigit - 1)));
                NumberResult += NumberTerminate;
                NumberDigit += 2;
            }
            return NumberResult;
        }

        static Fraction doCosineFraction(const Fraction &NumberSource, const Integer &NumberPrecision = Integer(10000000)) noexcept {
            Fraction NumberResult(1), NumberTerminate(1);
            unsigned short NumberDigit = 2;
            while (NumberTerminate.getAbsolute().doCompare({1, NumberPrecision}) > 0) {
                NumberTerminate = NumberTerminate.doMultiplication(NumberSource.doMultiplication(NumberSource).doDivision(Integer(NumberDigit * (NumberDigit - 1))).getOpposite());
                NumberResult = NumberResult.doAddition(NumberTerminate);
                NumberDigit += 2;
            }
            return NumberResult;
        }

        template<std::floating_point T>
        static T doEvolution(T NumberBase, T NumberPower) noexcept {
            return doPower(NumberBase, 1.0 / NumberPower);
        }

        static Fraction doEvolutionFraction(const Fraction &NumberBase, const Fraction &NumberPower) noexcept {
            return doPowerFraction(NumberBase, Fraction(1, 1).doDivision(NumberPower));
        }

        template<std::floating_point T>
        static T doExponent(T NumberSource) noexcept {
            T NumberDenominator = 1, NumberNumerator = NumberSource, NumberResult = 1, NumberTerminate = NumberSource;
            unsigned short NumberDigit = 1;
            while (std::abs(NumberTerminate) > 1e-12) {
                NumberResult += NumberTerminate;
                NumberNumerator *= NumberSource;
                NumberDenominator *= ++NumberDigit;
                NumberTerminate = NumberNumerator / NumberDenominator;
            }
            return NumberResult;
        }

        static Fraction doExponentFraction(const Fraction &NumberSource, const Integer &NumberPrecision = Integer(10000000)) noexcept {
            Fraction NumberDenominator(1), NumberNumerator = NumberSource, NumberResult(1), NumberTerminate = NumberSource;
            unsigned short NumberDigit = 1;
            while (NumberTerminate.getAbsolute().doCompare({1, NumberPrecision}) > 0) {
                NumberResult = NumberResult.doAddition(NumberTerminate);
                NumberNumerator = NumberNumerator.doMultiplication(NumberSource);
                NumberDenominator = NumberDenominator.doMultiplication(Integer(++NumberDigit));
                NumberTerminate = NumberNumerator.doDivision(NumberDenominator);
            }
            return NumberResult;
        }

        template<std::floating_point T>
        static T doHyperbolicCosine(T NumberSource) noexcept {
            return (doExponent(NumberSource) + doExponent(-NumberSource)) / 2.0;
        }

        static Fraction doHyperbolicCosineFraction(const Fraction &NumberSource) noexcept {
            return doExponentFraction(NumberSource).doAddition(doExponentFraction(NumberSource.getOpposite())).doDivision(Integer(2));
        }

        template<std::floating_point T>
        static T doHyperbolicSine(T NumberSource) noexcept {
            return (doExponent(NumberSource) - doExponent(-NumberSource)) / 2.0;
        }

        static Fraction doHyperbolicSineFraction(const Fraction &NumberSource) noexcept {
            return doExponentFraction(NumberSource).doSubtraction(doExponentFraction(NumberSource.getOpposite())).doDivision(Integer(2));
        }

        template<std::floating_point T>
        static T doHyperbolicTangent(T NumberSource) noexcept {
            return doHyperbolicSine(NumberSource) / doHyperbolicCosine(NumberSource);
        }

        static Fraction doHyperbolicTangentFraction(const Fraction &NumberSource) noexcept {
            return doHyperbolicSineFraction(NumberSource).doDivision(doHyperbolicCosineFraction(NumberSource));
        }

        template<std::floating_point T>
        static T doInverseHyperbolicTangent(T NumberDegree) noexcept {
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

        static Fraction doInverseHyperbolicTangentFraction(const Fraction &NumberDegree, const Integer &NumberPrecision = Integer(10000000)) noexcept {
            Fraction NumberNumerator(NumberDegree), NumberResult(0), NumberTerminate(NumberDegree);
            unsigned short NumberDigit = 1;
            while (NumberTerminate.getAbsolute().doCompare({1, NumberPrecision}) > 0) {
                NumberResult = NumberResult.doAddition(NumberTerminate);
                NumberNumerator = NumberNumerator.doMultiplication(NumberDegree.doMultiplication(NumberDegree));
                NumberDigit += 2;
                NumberTerminate = NumberNumerator.doDivision(Integer(NumberDigit));
            }
            return NumberResult;
        }

        template<std::floating_point T>
        static T doLogarithmE(T NumberSource) noexcept {
            return doInverseHyperbolicTangent((NumberSource - 1) / (NumberSource + 1)) * 2;
        }

        static Fraction doLogarithmEFraction(const Fraction &NumberSource) noexcept {
            return doInverseHyperbolicTangentFraction(NumberSource.doSubtraction(Integer(1)).doDivision(NumberSource.doAddition(Integer(1)))).doMultiplication(Integer(2));
        }

        template<std::floating_point T>
        static T doPower(T NumberBase, T NumberExponent) noexcept {
            return doExponent(doLogarithmE(NumberBase) * NumberExponent);
        }

        template<std::integral T>
        static T doPower(T NumberBase, T NumberExponent, T NumberModulo) noexcept {
            T NumberResult = 1;
            while (NumberExponent) {
                if (NumberExponent & 1) NumberResult = NumberResult * NumberBase % NumberModulo;
                NumberBase = NumberBase * NumberBase % NumberModulo;
                NumberExponent >>= 1;
            }
            return NumberResult;
        }

        static Fraction doPowerFraction(const Fraction &NumberBase, const Fraction &NumberExponent) noexcept {
            return doExponentFraction(doLogarithmEFraction(NumberBase).doMultiplication(NumberExponent));
        }

        template<std::floating_point T>
        static T doSine(T NumberSource) noexcept {
            NumberSource -= (int)std::abs(NumberSource / (std::numbers::pi * 2)) * 2 * std::numbers::pi;
            if (!NumberSource) return 0;
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

        template<std::floating_point T>
        static T doTangent(T NumberSource) noexcept {
            return doSine(NumberSource) / doCosine(NumberSource);
        }

        template<std::integral T>
        static T getAbsolute(T NumberSource) noexcept {
            return NumberSource >= 0 ? NumberSource : -NumberSource;
        }

        static Integer getGreatestCommonFactor(const Integer &Number1, const Integer &Number2) noexcept {
            if (!Number2.getAbsolute().doCompare(0)) return Number1;
            return getGreatestCommonFactor(Number2, Number1.doModulo(Number2));
        }

        static Integer getJocabiSymbol(const Integer &NumberMSource, const Integer &NumberNSource) {
            if (NumberNSource.isNegative() || NumberNSource.isEven()) throw Exception(String(u"Mathematics::getJocabiSymbol(const Integer&, const Integer&) NumberNSource"));
            Integer NumberM(NumberMSource), NumberN(NumberNSource);
            if (NumberM.isNegative() || NumberM.doCompare(NumberN) > 0) NumberM = NumberM.doModulo(NumberN);
            if (!NumberM.doCompare(0)) return NumberNSource.doCompare(1) == 0;
            if (Mathematics::getGreatestCommonFactor(NumberM, NumberN).doCompare(1)) return 0;
            Integer NumberJ(1);
            if (NumberM.isNegative()) {
                NumberM = NumberM.getOpposite();
                if (!NumberN.doModulo(4).doCompare(3)) NumberJ = NumberJ.getOpposite();
            }
            while (NumberM.doCompare(0)) {
                while (NumberM.isEven() && NumberM.isPositive()) {
                    NumberM = NumberM.doDivision(2);
                    if (!NumberN.doModulo(8).doCompare(3) || !NumberN.doModulo(8).doCompare(5))
                        NumberJ = NumberJ.getOpposite();
                }
                std::swap(NumberM, NumberN);
                if (!NumberM.doModulo(4).doCompare(3) && !NumberN.doModulo(4).doCompare(3)) NumberJ = NumberJ.getOpposite();
                NumberM = NumberM.doModulo(NumberN);
            }
            if (NumberN.doCompare(1)) NumberJ = 0;
            return NumberJ;
        }

        template<std::unsigned_integral T>
        static T getNearestPower2Lower(T NumberSource) noexcept {
            NumberSource |= NumberSource >> 1;
            NumberSource |= NumberSource >> 2;
            NumberSource |= NumberSource >> 4;
            NumberSource |= NumberSource >> 8;
            return (NumberSource + 1) >> 1;
        }

        template<std::unsigned_integral T>
        static T getNearestPower2Upper(T NumberSource) noexcept {
            NumberSource -= 1;
            NumberSource |= NumberSource >> 1;
            NumberSource |= NumberSource >> 2;
            NumberSource |= NumberSource >> 4;
            NumberSource |= NumberSource >> 8;
            NumberSource |= NumberSource >> 16;
            return (NumberSource < 0) ? 1 : NumberSource + 1;
        }

        template<std::unsigned_integral T>
        static T getTrailingZeroCount(T NumberSource) noexcept {
            constexpr int ConvertTable[64] = {
                0, 1, 2, 53, 3, 7, 54, 27, 4, 38, 41, 8, 34, 55, 48, 28,
                62, 5, 39, 46, 44, 42, 22, 9, 24, 35, 59, 56, 49, 18, 29, 11,
                63, 52, 6, 26, 37, 40, 33, 47, 61, 45, 43, 21, 23, 58, 17, 10,
                51, 25, 36, 32, 60, 20, 57, 16, 50, 31, 19, 15, 30, 14, 13, 12};
            return ConvertTable[(NumberSource & ~(NumberSource - 1)) * 0x022FDD63CC95386D >> 58];
        }

        static Integer getTrailingZeroCount(const Integer &NumberSourceSource) noexcept {
            Integer NumberCount(0), NumberSource(NumberSourceSource);
            while (NumberSource.isEven()) NumberSource = NumberSource.doDivision(2), NumberCount = NumberCount.doAddition(1);
            return NumberCount;
        }

        static bool isPrime(const Integer &NumberSource) noexcept {
            if (NumberSource.doCompare(2) < 0) return false;
            if (!NumberSource.doCompare(2) || !NumberSource.doCompare(3) || !NumberSource.doCompare(5)) return true;
            if (NumberSource.isEven() || !NumberSource.doModulo(3).doCompare(0) || !NumberSource.doModulo(5).doCompare(0))
                return false;
            if (NumberSource.doCompare(49) < 0) return true;
            if (!NumberSource.doModulo(7).doCompare(0) || !NumberSource.doModulo(11).doCompare(0) ||
                !NumberSource.doModulo(13).doCompare(0) || !NumberSource.doModulo(17).doCompare(0) ||
                !NumberSource.doModulo(19).doCompare(0) || !NumberSource.doModulo(23).doCompare(0) ||
                !NumberSource.doModulo(29).doCompare(0) || !NumberSource.doModulo(31).doCompare(0) ||
                !NumberSource.doModulo(37).doCompare(0) || !NumberSource.doModulo(41).doCompare(0) ||
                !NumberSource.doModulo(43).doCompare(0) || !NumberSource.doModulo(47).doCompare(0))
                return false;
            if (NumberSource.doCompare(2809) < 0) return true;
            if (NumberSource.doCompare(23001) <= 0)
                return Integer(2).doPower(NumberSource, NumberSource).doCompare(2) == 0 && NumberSource.doCompare(7957) && NumberSource.doCompare(8321) && NumberSource.doCompare(13747) && NumberSource.doCompare(18721) && NumberSource.doCompare(19951);
            if (NumberSource.doCompare(341531) < 0) return isPrimeRabinMiller(NumberSource, {String(u"9345883071009581737")});
            if (NumberSource.doCompare(885594169) < 0) return isPrimeRabinMiller(NumberSource, {725270293939359937, 3569819667048198375});
            if (NumberSource.doCompare(350269456337) < 0) return isPrimeRabinMiller(NumberSource, {4230279247111683200, String(u"14694767155120705706"), String(u"16641139526367750375")});
            if (NumberSource.doCompare(55245642489451) < 0) return isPrimeRabinMiller(NumberSource, {2, 141889084524735, 1199124725622454117, String(u"11096072698276303650")});
            if (NumberSource.doCompare(7999252175582851) < 0) return isPrimeRabinMiller(NumberSource, {2, 4130806001517, 149795463772692060, 186635894390467037, 3967304179347715805});
            if (NumberSource.doCompare(585226005592931977) < 0) return isPrimeRabinMiller(NumberSource, {2, 123635709730000, 9233062284813009, 43835965440333360, 761179012939631437, 1263739024124850375});
            if (NumberSource.doCompare(String(u"18446744073709551616")) < 0) return isPrimeRabinMiller(NumberSource, {2, 325, 9375, 28178, 450775, 9780504, 1795265022});
            return isPrimeRabinMiller(NumberSource, {2}) && isPrimeLucas(NumberSource);
        }

        template<std::unsigned_integral T>
        static bool isPrimeNative(T NumberSource) noexcept {
            if (NumberSource <= 1) return false;
            if (NumberSource <= 3) return true;
            if (!(NumberSource & 1) || ((NumberSource - 1) % 6 && (NumberSource + 1) % 6)) return false;
            for (T NumberFactor = 3;NumberFactor * NumberFactor <= NumberSource;NumberFactor += 2)
                if (NumberSource % NumberFactor == 0) return false;
            return true;
        }

        template<typename T> requires std::is_arithmetic<T>::value
        static T toDegrees(T NumberSource) noexcept {
            return 180.0 / std::numbers::pi * NumberSource;
        }

        template<typename T> requires std::is_arithmetic<T>::value
        static T toRadians(T NumberSource) noexcept {
            return std::numbers::pi / 180.0 * NumberSource;
        }
    };
}
