#pragma once

#ifndef eLibraryHeaderCoreMathematics
#define eLibraryHeaderCoreMathematics

#include <Core/Number.hpp>
#include <numbers>

namespace eLibrary::Core {
    class MathematicsContext final : public Object {
    private:
        Integer FunctionPrecision;
    public:
        explicit MathematicsContext(const Integer &FunctionPrecisionSource) noexcept : FunctionPrecision(FunctionPrecisionSource) {}

        static MathematicsContext getDefault() noexcept {
            static MathematicsContext ContextObject{10000000};
            return ContextObject;
        }

        Integer getFunctionPrecision() const noexcept {
            return FunctionPrecision;
        }

        void setFunctionPrecision(const Integer &FunctionPrecisionSource) noexcept {
            FunctionPrecision = FunctionPrecisionSource;
        }
    };

    /**
     * Support for mathematical operations
     */
    class Mathematics final : public NonConstructable {
    private:
        static bool isPrimeLucas(const Integer &NumberSource) noexcept {
            Integer NumberD(5);
            for (;;) {
                Integer NumberG(NumberD.getAbsolute().getGreatestCommonFactor(NumberSource));
                if (NumberG.doCompare(1) > 0 && NumberG.doCompare(NumberSource)) return false;
                if (!getJocabiSymbol(NumberD, NumberSource).doCompare(-1)) break;
                if (NumberD.isPositive()) NumberD = NumberD.getOpposite().doSubtraction(2);
                else NumberD = NumberD.getOpposite().doAddition(2);
            }
            if (NumberD.isZero()) return false;
            Integer NumberIterationCount(0), NumberK(NumberSource.doAddition(1)), NumberP(1), NumberQ((1_I).doSubtraction(NumberD).doDivision(4));
            while (NumberK.isEven()) NumberK = NumberK.doDivision(2), NumberIterationCount = NumberIterationCount.doAddition(1);
            Integer NumberBitCount(NumberK.toString(2).getCharacterSize()), NumberQk(NumberQ), NumberU(1), NumberV(NumberP);
            if (!NumberQ.doCompare(1)) {
                while (NumberBitCount.doCompare(1) > 0) {
                    NumberU = NumberU.doMultiplication(NumberV).doModulo(NumberSource);
                    NumberV = NumberV.doMultiplication(NumberV).doSubtraction(2).doModulo(NumberSource);
                    NumberBitCount = NumberBitCount.doSubtraction(1);
                    if (NumberK.doDivision((2_I).doPower(NumberBitCount.doSubtraction(1))).isOdd()) {
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
                    if (NumberK.doDivision((2_I).doPower(NumberBitCount.doSubtraction(1))).isOdd()) {
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
                    if (NumberK.doDivision((2_I).doPower(NumberBitCount.doSubtraction(1))).isOdd()) {
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
            if (NumberU.isZero() || NumberV.isZero()) return true;
            for (Integer NumberIteration = 1;NumberIteration.doCompare(NumberIterationCount) < 0;NumberIteration = NumberIteration.doAddition(1)) {
                NumberV = NumberV.doMultiplication(NumberV).doSubtraction(NumberQk.doAddition(NumberQk)).doModulo(NumberSource);
                if (NumberV.isZero()) return true;
                NumberQk = NumberQk.doPower(2, NumberSource);
            }
            return false;
        }

        static bool isPrimeRabinMiller(const Integer &NumberSource, ::std::initializer_list<Integer> NumberBaseList) noexcept {
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
        inline static const Fraction NumberPi = {52163, 16604};

        constexpr Mathematics() noexcept = delete;

        template<std::unsigned_integral T>
        static T doCeil2(T NumberSource) noexcept {
            NumberSource -= 1;
            NumberSource |= NumberSource >> 1;
            NumberSource |= NumberSource >> 2;
            NumberSource |= NumberSource >> 4;
            NumberSource |= NumberSource >> 8;
            NumberSource |= NumberSource >> 16;
            return (NumberSource < 0) ? 1 : NumberSource + 1;
        }

        static Integer doCombinator(const Integer &NumberM, const Integer &NumberN) {
            if (NumberN.doCompare(NumberM) < 0) doThrowChecked(ArithmeticException, u"Mathematics::doCombinator(const Integer&, const Integer&) NumberM NumberN"_S);
            return NumberN.doFactorial().doDivision(NumberM.doFactorial().doMultiplication(NumberN.doSubtraction(NumberM).doFactorial()));
        }

        static Fraction doCosecantFraction(const Fraction &NumberSource, const MathematicsContext &NumberContext) noexcept {
            return Fraction(1).doDivision(doSineFraction(NumberSource, NumberContext));
        }

        static Fraction doCosineFraction(const Fraction &NumberSourceSource, const MathematicsContext &NumberContext) noexcept {
            Fraction NumberSource(NumberSourceSource.doSubtraction(Fraction(NumberSourceSource.doDivision(NumberPi.doAddition(NumberPi)).toInteger()).doMultiplication(2).doMultiplication(NumberPi)));
            Fraction NumberResult(1), NumberTerminate(1);
            unsigned short NumberDigit = 2;
            while (NumberTerminate.getAbsolute().doCompare({1, NumberContext.getFunctionPrecision()}) > 0) {
                NumberTerminate = NumberTerminate.doMultiplication(NumberSource.doMultiplication(NumberSource).doDivision(Integer(NumberDigit * (NumberDigit - 1))).getOpposite());
                NumberResult = NumberResult.doAddition(NumberTerminate);
                NumberDigit += 2;
            }
            return NumberResult;
        }

        static Fraction doCotangentFraction(const Fraction &NumberSource, const MathematicsContext &NumberContext) noexcept {
            return doCosineFraction(NumberSource, NumberContext).doDivision(doSineFraction(NumberSource, NumberContext));
        }

        static Fraction doEvolutionFraction(const Fraction &NumberBase, const Fraction &NumberPower, const MathematicsContext &NumberContext) noexcept {
            return doPowerFraction(NumberBase, Fraction(1, 1).doDivision(NumberPower), NumberContext);
        }

        static Fraction doExponentFraction(const Fraction &NumberSource, const MathematicsContext &NumberContext) noexcept {
            Fraction NumberDenominator(1), NumberNumerator = NumberSource, NumberResult(1), NumberTerminate = NumberSource;
            unsigned short NumberDigit = 1;
            while (NumberTerminate.getAbsolute().doCompare({1, NumberContext.getFunctionPrecision()}) > 0) {
                NumberResult = NumberResult.doAddition(NumberTerminate);
                NumberNumerator = NumberNumerator.doMultiplication(NumberSource);
                NumberDenominator = NumberDenominator.doMultiplication(Integer(++NumberDigit));
                NumberTerminate = NumberNumerator.doDivision(NumberDenominator);
            }
            return NumberResult;
        }

        template<std::unsigned_integral T>
        static T doFloor2(T NumberSource) noexcept {
            NumberSource |= NumberSource >> 1;
            NumberSource |= NumberSource >> 2;
            NumberSource |= NumberSource >> 4;
            NumberSource |= NumberSource >> 8;
            return (NumberSource + 1) >> 1;
        }

        static Fraction doHyperbolicCosineFraction(const Fraction &NumberSource, const MathematicsContext &NumberContext) noexcept {
            return doExponentFraction(NumberSource, NumberContext).doAddition(doExponentFraction(NumberSource.getOpposite(), NumberContext)).doDivision(2_I);
        }

        static Fraction doHyperbolicSineFraction(const Fraction &NumberSource, const MathematicsContext &NumberContext) noexcept {
            return doExponentFraction(NumberSource, NumberContext).doSubtraction(doExponentFraction(NumberSource.getOpposite(), NumberContext)).doDivision(2_I);
        }

        static Fraction doHyperbolicTangentFraction(const Fraction &NumberSource, const MathematicsContext &NumberContext) noexcept {
            return doHyperbolicSineFraction(NumberSource, NumberContext).doDivision(doHyperbolicCosineFraction(NumberSource, NumberContext));
        }

        static Fraction doInverseHyperbolicTangentFraction(const Fraction &NumberDegree, const MathematicsContext &NumberContext) noexcept {
            Fraction NumberNumerator(NumberDegree), NumberResult(0), NumberTerminate(NumberDegree);
            unsigned short NumberDigit = 1;
            while (NumberTerminate.getAbsolute().doCompare({1, NumberContext.getFunctionPrecision()}) > 0) {
                NumberResult = NumberResult.doAddition(NumberTerminate);
                NumberNumerator = NumberNumerator.doMultiplication(NumberDegree.doMultiplication(NumberDegree));
                NumberDigit += 2;
                NumberTerminate = NumberNumerator.doDivision(Integer(NumberDigit));
            }
            return NumberResult;
        }

        static Fraction doLogarithmEFraction(const Fraction &NumberSource, const MathematicsContext &NumberContext) noexcept {
            return doInverseHyperbolicTangentFraction(NumberSource.doSubtraction(1_I).doDivision(NumberSource.doAddition(1_I)), NumberContext).doMultiplication(2_I);
        }

        template<std::integral T>
        static T doPower(T NumberBase, T NumberExponent) noexcept {
            T NumberResult = 1;
            while (NumberExponent) {
                if (NumberExponent & 1) NumberResult = NumberResult * NumberBase;
                NumberBase = NumberBase * NumberBase;
                NumberExponent >>= 1;
            }
            return NumberResult;
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

        static Fraction doPowerFraction(const Fraction &NumberBase, const Fraction &NumberExponent, const MathematicsContext &NumberContext) noexcept {
            return doExponentFraction(doLogarithmEFraction(NumberBase, NumberContext).doMultiplication(NumberExponent), NumberContext);
        }

        static Fraction doSecantFraction(const Fraction &NumberSource, const MathematicsContext &NumberContext) noexcept {
            return Fraction(1, 1).doDivision(doCosineFraction(NumberSource, NumberContext));
        }

        static Fraction doSineFraction(const Fraction &NumberSourceSource, const MathematicsContext &NumberContext) noexcept {
            Fraction NumberSource(NumberSourceSource.doSubtraction(Fraction(NumberSourceSource.doDivision(NumberPi.doAddition(NumberPi)).toInteger()).doMultiplication(2).doMultiplication(NumberPi)));
            Fraction NumberDenominator(1), NumberNumerator(NumberSource), NumberResult(0), NumberSignature(1), NumberTerminate(NumberSource);
            unsigned short NumberDigit = 1;
            while (NumberTerminate.getAbsolute().doCompare({1, NumberContext.getFunctionPrecision()}) > 0) {
                NumberResult = NumberResult.doAddition(NumberTerminate);
                ++NumberDigit;
                NumberSignature = NumberSignature.getOpposite();
                NumberDenominator = NumberDenominator.doMultiplication(Integer(((NumberDigit << 1) - 2) * ((NumberDigit << 1) - 1)));
                NumberNumerator = NumberSource.doMultiplication(NumberSource).doMultiplication(NumberNumerator);
                NumberTerminate = NumberSignature.doMultiplication(NumberNumerator.doDivision(NumberDenominator));
            }
            return NumberResult;
        }

        static Fraction doTangentFraction(const Fraction &NumberSource, const MathematicsContext &NumberContext) noexcept {
            return doSineFraction(NumberSource, NumberContext).doDivision(doCosineFraction(NumberSource, NumberContext));
        }

        template<Type::Arithmetic T>
        static T getAbsolute(T NumberSource) noexcept {
            return NumberSource >= 0 ? NumberSource : -NumberSource;
        }

        static Integer getJocabiSymbol(const Integer &NumberMSource, const Integer &NumberNSource) noexcept {
            if (NumberNSource.isNegative() || NumberNSource.isEven()) doThrowUnchecked(ArithmeticException(u"Mathematics::getJocabiSymbol(const Integer&, const Integer&) NumberNSource"_S));
            Integer NumberM(NumberMSource), NumberN(NumberNSource);
            if (NumberM.isNegative() || NumberM.doCompare(NumberN) > 0) NumberM = NumberM.doModulo(NumberN);
            if (NumberM.isZero()) return !NumberNSource.doCompare(1);
            if (NumberM.getGreatestCommonFactor(NumberN).doCompare(1)) return 0;
            Integer NumberJ(1);
            if (NumberM.isNegative()) {
                NumberM = NumberM.getOpposite();
                if (!NumberN.doModulo(4).doCompare(3)) NumberJ = NumberJ.getOpposite();
            }
            while (!NumberM.isZero()) {
                while (NumberM.isEven() && NumberM.isPositive()) {
                    NumberM = NumberM.doDivision(2);
                    if (!NumberN.doModulo(8).doCompare(3) || !NumberN.doModulo(8).doCompare(5))
                        NumberJ = NumberJ.getOpposite();
                }
                Objects::doSwap(NumberM, NumberN);
                if (!NumberM.doModulo(4).doCompare(3) && !NumberN.doModulo(4).doCompare(3)) NumberJ = NumberJ.getOpposite();
                NumberM = NumberM.doModulo(NumberN);
            }
            if (NumberN.doCompare(1)) NumberJ = 0;
            return NumberJ;
        }

        template<std::unsigned_integral T>
        static T getTrailingZeroCount(T NumberSource) noexcept {
            static constexpr int ConvertTable[] = {
                0, 1, 2, 53, 3, 7, 54, 27, 4, 38, 41, 8, 34, 55, 48, 28,
                62, 5, 39, 46, 44, 42, 22, 9, 24, 35, 59, 56, 49, 18, 29, 11,
                63, 52, 6, 26, 37, 40, 33, 47, 61, 45, 43, 21, 23, 58, 17, 10,
                51, 25, 36, 32, 60, 20, 57, 16, 50, 31, 19, 15, 30, 14, 13, 12};
            return ConvertTable[(NumberSource & ~(NumberSource - 1)) * 0x22FDD63CC95386D >> 58];
        }

        static Integer getTrailingZeroCount(const Integer &NumberSourceSource) noexcept {
            Integer NumberCount(0), NumberSource(NumberSourceSource);
            while (NumberSource.isEven()) NumberSource = NumberSource.doDivision(2), NumberCount = NumberCount.doAddition(1);
            return NumberCount;
        }

        static bool isPrime(const Integer &NumberSource) noexcept {
            if (NumberSource.doCompare(2) < 0) return false;
            if (!NumberSource.doCompare(2) || !NumberSource.doCompare(3) || !NumberSource.doCompare(5)) return true;
            if (NumberSource.isEven() || NumberSource.doModulo(3).isZero() || NumberSource.doModulo(5).isZero())
                return false;
            if (NumberSource.doCompare(49) < 0) return true;
            if (NumberSource.doModulo(7).isZero() || NumberSource.doModulo(11).isZero() ||
                NumberSource.doModulo(13).isZero() || NumberSource.doModulo(17).isZero() ||
                NumberSource.doModulo(19).isZero() || NumberSource.doModulo(23).isZero() ||
                NumberSource.doModulo(29).isZero() || NumberSource.doModulo(31).isZero() ||
                NumberSource.doModulo(37).isZero() || NumberSource.doModulo(41).isZero() ||
                NumberSource.doModulo(43).isZero() || NumberSource.doModulo(47).isZero())
                return false;
            if (NumberSource.doCompare(2809) < 0) return true;
            if (NumberSource.doCompare(23001) <= 0)
                return !(2_I).doPower(NumberSource, NumberSource).doCompare(2) && NumberSource.doCompare(7957) && NumberSource.doCompare(8321) && NumberSource.doCompare(13747) && NumberSource.doCompare(18721) && NumberSource.doCompare(19951);
            if (NumberSource.doCompare(341531) < 0) return isPrimeRabinMiller(NumberSource, {u"9345883071009581737"_I});
            if (NumberSource.doCompare(885594169) < 0) return isPrimeRabinMiller(NumberSource, {725270293939359937, 3569819667048198375});
            if (NumberSource.doCompare(350269456337) < 0) return isPrimeRabinMiller(NumberSource, {4230279247111683200, u"14694767155120705706"_I, u"16641139526367750375"_I});
            if (NumberSource.doCompare(55245642489451) < 0) return isPrimeRabinMiller(NumberSource, {2, 141889084524735, 1199124725622454117, u"11096072698276303650"_I});
            if (NumberSource.doCompare(7999252175582851) < 0) return isPrimeRabinMiller(NumberSource, {2, 4130806001517, 149795463772692060, 186635894390467037, 3967304179347715805});
            if (NumberSource.doCompare(585226005592931977) < 0) return isPrimeRabinMiller(NumberSource, {2, 123635709730000, 9233062284813009, 43835965440333360, 761179012939631437, 1263739024124850375});
            if (NumberSource.doCompare(u"18446744073709551616"_I) < 0) return isPrimeRabinMiller(NumberSource, {2, 325, 9375, 28178, 450775, 9780504, 1795265022});
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

        template<Type::Arithmetic T>
        static T toDegrees(T NumberSource) noexcept {
            return 180. / ::std::numbers::pi * NumberSource;
        }

        static Fraction toDegreesFraction(const Fraction &NumberSource) noexcept {
            return Fraction(180).doDivision(NumberPi).doMultiplication(NumberSource);
        }

        template<Type::Arithmetic T>
        static T toRadians(T NumberSource) noexcept {
            return ::std::numbers::pi / 180. * NumberSource;
        }

        static Fraction toRadiansFraction(const Fraction &NumberSource) noexcept {
            return NumberPi.doDivision({180}).doMultiplication(NumberSource);
        }
    };
}

#endif
