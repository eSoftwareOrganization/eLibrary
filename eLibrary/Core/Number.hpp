#pragma once

#ifndef eLibraryHeaderCoreNumber
#define eLibraryHeaderCoreNumber

#include <Core/Exception.hpp>
#include <limits>
#include <vector>

namespace eLibrary::Core {
    class Numbers final : public Object, public NonConstructable {
    public:
        template<std::floating_point T>
        static intmax_t doCompare(T Number1, T Number2) noexcept {
            if (Number1 - Number2 > ::std::numeric_limits<T>::epsilon()) return 1;
            if (Number1 - Number2 < -std::numeric_limits<T>::epsilon()) return -1;
            return 0;
        }

        template<std::integral T>
        static intmax_t doCompare(T Number1, T Number2) noexcept {
            if (Number1 > Number2) return 1;
            if (Number1 < Number2) return -1;
            return 0;
        }
    };

    /**
     * Support for integral operation and storage
     */
    class Integer final : public Object {
    private:
        class NumberVector final : protected ::std::vector<intmax_t> {
        public:
            NumberVector() noexcept = default;

            NumberVector(intmax_t NumberSize, intmax_t NumberValue) : ::std::vector<intmax_t>(NumberSize, NumberValue) {}

            void addElement(intmax_t NumberSource) {
                push_back(NumberSource);
            }

            NumberVector doMultiplication(const NumberVector &NumberOther) const {
                NumberVector NumberProduct(getElementSize() + NumberOther.getElementSize(), 0);
                for (intmax_t NumberDigit1 = 0; NumberDigit1 < getElementSize(); ++NumberDigit1)
                    for (intmax_t NumberDigit2 = 0; NumberDigit2 < NumberOther.getElementSize(); ++NumberDigit2)
                        NumberProduct[NumberDigit1 + NumberDigit2] += NumberOther[NumberDigit1] * NumberOther[NumberDigit2];
                NumberVector NumberResult;
                NumberResult.doReserve(getElementSize() + NumberOther.getElementSize());
                intmax_t NumberCarry = 0;
                for (intmax_t NumberPart = 0;; ++NumberPart) {
                    if (!NumberCarry && NumberPart >= getElementSize() + NumberOther.getElementSize()) break;
                    intmax_t NumberCurrent = NumberProduct[NumberPart] + NumberCarry;
                    NumberResult.addElement(NumberCurrent % 10000000);
                    NumberCarry = NumberCurrent / 10000000;
                }
                NumberResult.doRemove0();
                return NumberResult;
            }

            void doReserve(intmax_t NumberSize) {
                reserve(NumberSize);
            }

            intmax_t getElementFront() const noexcept {
                return front();
            }

            intmax_t getElementSize() const noexcept {
                return (intmax_t) size();
            }

            bool isEmpty() const noexcept {
                return empty();
            }

            intmax_t &operator[](intmax_t NumberIndex) noexcept {
                return ::std::vector<intmax_t>::operator[](NumberIndex);
            }

            intmax_t operator[](intmax_t NumberIndex) const noexcept {
                return ::std::vector<intmax_t>::operator[](NumberIndex);
            }

            void doRemove0() noexcept {
                while (!back() && getElementSize() > 1)
                    pop_back();
            }
        };

        bool NumberSignature;
        NumberVector NumberList;

        static Integer doFactorialCore(const Integer &NumberStart, const Integer &NumberCount) noexcept {
            if (NumberCount.doCompare(8) < 0) {
                Integer NumberResult(1);
                for (Integer NumberCurrent(NumberStart);NumberCurrent.doCompare(NumberStart.doAddition(NumberCount)) < 0;NumberCurrent = NumberCurrent.doAddition(1))
                    NumberResult = NumberResult.doMultiplication(NumberCurrent);
                return NumberResult;
            }
            Integer NumberMiddle(NumberCount.doAddition(1).doDivision(2));
            return doFactorialCore(NumberStart, NumberMiddle).doMultiplication(doFactorialCore(NumberStart.doAddition(NumberMiddle), NumberCount.doSubtraction(NumberMiddle)));
        }

        template<std::signed_integral T>
        static T getAbsolute(T NumberSource) noexcept {
            return NumberSource >= 0 ? NumberSource : -NumberSource;
        }

        Integer(const NumberVector &NumberListSource, bool NumberSignatureSource) noexcept : NumberSignature(NumberSignatureSource), NumberList(NumberListSource) {}

        Integer doMultiplicationAbsolute(const Integer &NumberOther) const {
            return {NumberList.doMultiplication(NumberOther.NumberList), true};
        }

        friend class Fraction;

        friend Integer operator%(const Integer &Number1, const Integer &Number2) {
            return Number1.doModulo(Number2);
        }

        friend Integer operator*(const Integer &Number1, const Integer &Number2) noexcept {
            return Number1.doMultiplication(Number2);
        }

        friend Integer operator+(const Integer &Number1, const Integer &Number2) noexcept {
            return Number1.doAddition(Number2);
        }

        friend Integer operator-(const Integer &Number1, const Integer &Number2) noexcept {
            return Number1.doSubtraction(Number2);
        }

        friend Integer operator-(const Integer &NumberSource) noexcept {
            return NumberSource.getOpposite();
        }

        friend Integer operator/(const Integer &Number1, const Integer &Number2) {
            return Number1.doDivision(Number2);
        }
    public:
        Integer() noexcept : NumberSignature(true) {
            NumberList.addElement(0);
        }

        template<::std::signed_integral T>
        Integer(T NumberValueSource) noexcept : NumberSignature(NumberValueSource >= 0) {
            intmax_t NumberValue(NumberValueSource);
            do {
                NumberList.addElement(getAbsolute(NumberValue % 10000000));
                NumberValue /= 10000000;
            } while (NumberValue);
        }

        template<::std::unsigned_integral T>
        Integer(T NumberValueSource) noexcept : NumberSignature(true) {
            uintmax_t NumberValue(NumberValueSource);
            do {
                NumberList.addElement(NumberValue % 10000000);
                NumberValue /= 10000000;
            } while (NumberValue);
        }

        Integer(const String &NumberValue, uint8_t NumberRadix) : NumberSignature(true) {
            if (NumberRadix < 2 || NumberRadix > 36) [[unlikely]]
                throw ArithmeticException(u"Integer::Integer(const String&, uint8_t) NumberRadix"_S);
            if (NumberValue.isEmpty()) [[unlikely]]
                throw Exception(u"Integer::Integer(const String&, uint8_t) NumberValue"_S);
            Integer IntegerRadix(NumberRadix);
            intmax_t NumberDigit(0);
            if ((char16_t) NumberValue.getCharacter(0) == u'-') NumberSignature = false, NumberDigit = 1;
            else if ((char16_t) NumberValue.getCharacter(0) == u'+') NumberDigit = 1;
            for (; NumberDigit < NumberValue.getCharacterSize(); ++NumberDigit)
                NumberList = doMultiplication(IntegerRadix).doAddition(NumberValue.getCharacter(NumberDigit).toNumber(NumberRadix)).NumberList;
            if (NumberList.isEmpty()) NumberList.addElement(0);
            NumberList.doRemove0();
        }

        Integer doAddition(const Integer &NumberOther) const noexcept {
            if (NumberSignature && !NumberOther.NumberSignature) return doSubtraction(NumberOther.getAbsolute());
            if (!NumberSignature && NumberOther.NumberSignature) return NumberOther.doSubtraction(getAbsolute());
            NumberVector NumberResult;
            NumberResult.doReserve(Objects::getMaximum(NumberList.getElementSize(), NumberOther.NumberList.getElementSize()));
            intmax_t NumberCarry = 0;
            for (intmax_t NumberPart = 0;; ++NumberPart) {
                if (!NumberCarry && NumberPart >= NumberList.getElementSize() && NumberPart >= NumberOther.NumberList.getElementSize())
                    break;
                intmax_t NumberCurrent = NumberCarry;
                if (NumberPart < NumberList.getElementSize()) NumberCurrent += NumberList[NumberPart];
                if (NumberPart < NumberOther.NumberList.getElementSize()) NumberCurrent += NumberOther.NumberList[NumberPart];
                NumberResult.addElement(NumberCurrent % 10000000);
                NumberCarry = NumberCurrent / 10000000;
            }
            return {NumberResult, !(!NumberSignature && !NumberOther.NumberSignature)};
        }

        intmax_t doCompare(const Integer &NumberOther) const noexcept {
            if (NumberSignature != NumberOther.NumberSignature && (NumberList.getElementSize() != 1 || NumberList[0]) && (NumberOther.NumberList.getElementSize() != 1 || NumberList[0]))
                return Numbers::doCompare(NumberSignature, NumberOther.NumberSignature);
            if (NumberList.getElementSize() != NumberOther.NumberList.getElementSize())
                return NumberSignature ? Numbers::doCompare(NumberList.getElementSize(), NumberOther.NumberList.getElementSize()) : Numbers::doCompare(NumberOther.NumberList.getElementSize(), NumberList.getElementSize());
            for (auto NumberPart = NumberList.getElementSize() - 1; NumberPart >= 0; --NumberPart)
                if (NumberList[NumberPart] != NumberOther.NumberList[NumberPart])
                    return NumberSignature ? Numbers::doCompare(NumberList[NumberPart], NumberOther.NumberList[NumberPart]) : Numbers::doCompare(NumberOther.NumberList[NumberPart], NumberList[NumberPart]);
            return 0;
        }

        Integer doDivision(const Integer &NumberOther) const {
            if (NumberOther.isZero()) [[unlikely]]
                throw ArithmeticException(u"Integer::doDivision(const Integer&) Divide By 0"_S);
            Integer NumberRemainder;
            NumberVector NumberResult(NumberList);
            for (auto NumberPart = NumberList.getElementSize() - 1; NumberPart >= 0; --NumberPart) {
                NumberRemainder.NumberList = NumberRemainder.doMultiplication(10000000).doAddition(NumberList[NumberPart]).NumberList;
                uintmax_t NumberMiddle, NumberStart = 0, NumberStop = 9999999U;
                for (;;) {
                    NumberMiddle = (NumberStart + NumberStop) >> 1;
                    if (NumberOther.doMultiplicationAbsolute(NumberMiddle).doCompare(NumberRemainder) <= 0)
                        if (NumberOther.doMultiplicationAbsolute(NumberMiddle + 1).doCompare(NumberRemainder) > 0) break;
                        else NumberStart = NumberMiddle;
                    else NumberStop = NumberMiddle;
                }
                NumberResult[NumberPart] = NumberMiddle;
                NumberRemainder.NumberList = NumberRemainder.doSubtraction(NumberOther.doMultiplicationAbsolute(NumberMiddle)).NumberList;
            }
            NumberResult.doRemove0();
            return {NumberResult, !(NumberSignature ^ NumberOther.NumberSignature)};
        }

        Integer doFactorial(const Integer &NumberStep = {1}) const {
            if (!NumberStep.isPositive()) [[unlikely]]
                throw ArithmeticException(u"Integer::doFactorial(const Integer&={1}) NumberStep"_S);
            if (isNegative()) [[unlikely]]
                throw ArithmeticException(u"Integer::doFactorial(const Integer&={1}) isNegative"_S);
            if (!NumberStep.doCompare(1)) [[likely]]
                return doFactorialCore(1, *this);
            Integer NumberCurrent(NumberList, true);
            NumberVector NumberResult(1, 1);
            while (NumberCurrent.isPositive()) {
                NumberResult = NumberResult.doMultiplication(NumberList);
                NumberCurrent = NumberCurrent.doSubtraction(NumberStep);
            }
            return {NumberResult, true};
        }

        Integer doModulo(const Integer &NumberOther) const {
            if (NumberOther.isZero()) [[unlikely]]
                throw ArithmeticException(u"Integer::doModulo(const Integer&) Modulo By 0"_S);
            Integer NumberRemainder;
            NumberVector NumberResult(NumberList);
            for (auto NumberPart = NumberList.getElementSize() - 1; NumberPart >= 0; --NumberPart) {
                NumberRemainder.NumberList = NumberRemainder.doMultiplication(10000000).doAddition(NumberList[NumberPart]).NumberList;
                uintmax_t NumberMiddle, NumberStart = 0, NumberStop = 9999999U;
                for (;;) {
                    NumberMiddle = (NumberStart + NumberStop) >> 1;
                    if (NumberOther.doMultiplicationAbsolute(NumberMiddle).doCompare(NumberRemainder) <= 0)
                        if (NumberOther.doMultiplicationAbsolute(NumberMiddle + 1).doCompare(NumberRemainder) > 0) break;
                        else NumberStart = NumberMiddle;
                    else NumberStop = NumberMiddle;
                }
                NumberResult[NumberPart] = NumberMiddle;
                NumberRemainder.NumberList = NumberRemainder.doSubtraction(NumberOther.doMultiplicationAbsolute(NumberResult[NumberPart])).NumberList;
            }
            NumberRemainder.NumberSignature = NumberSignature;
            return NumberRemainder;
        }

        Integer doMultiplication(const Integer &NumberOther) const {
            return {NumberList.doMultiplication(NumberOther.NumberList), !(NumberSignature ^ NumberOther.NumberSignature)};
        }

        Integer doPower(const Integer &NumberExponentSource) const {
            if (isZero() && NumberExponentSource.isZero()) [[unlikely]]
                throw ArithmeticException(u"Integer::doPower(const Integer&, const Integer&) 0 to the power of 0"_S);
            if (NumberExponentSource.isNegative()) [[unlikely]]
                throw ArithmeticException(u"Integer::doPower(const Integer&) NumberExponentSource"_S);
            Integer NumberBase(*this), NumberExponent(NumberExponentSource), NumberResult(1);
            while (!NumberExponent.isZero()) {
                if (NumberExponent.isOdd()) NumberResult = NumberResult.doMultiplication(NumberBase);
                NumberBase = NumberBase.doMultiplication(NumberBase);
                NumberExponent = NumberExponent.doDivision(2);
            }
            return NumberResult;
        }

        Integer doPower(const Integer &NumberExponentSource, const Integer &NumberModulo) const {
            if (isZero() && NumberExponentSource.isZero()) [[unlikely]]
                throw ArithmeticException(u"Integer::doPower(const Integer&, const Integer&) 0 to the power of 0"_S);
            if (NumberExponentSource.isNegative()) [[unlikely]]
                throw ArithmeticException(u"Integer::doPower(const Integer&, const Integer&) NumberExponentSource"_S);
            Integer NumberBase(*this), NumberExponent(NumberExponentSource), NumberResult(1);
            while (!NumberExponent.isZero()) {
                if (NumberExponent.isOdd()) NumberResult = NumberResult.doMultiplication(NumberBase).doModulo(NumberModulo);
                NumberBase = NumberBase.doMultiplication(NumberBase).doModulo(NumberModulo);
                NumberExponent = NumberExponent.doDivision(2);
            }
            return NumberResult;
        }

        Integer doSubtraction(const Integer &NumberOther) const noexcept {
            if (NumberOther.isNegative()) return doAddition(NumberOther.getAbsolute());
            if (doCompare(NumberOther) < 0) return NumberOther.doSubtraction(*this).getOpposite();
            NumberVector NumberResult;
            NumberResult.doReserve(Objects::getMaximum(NumberList.getElementSize(), NumberOther.NumberList.getElementSize()) + 1);
            bool NumberBorrow = false;
            for (intmax_t NumberPart = 0;; ++NumberPart) {
                if (!NumberBorrow && NumberPart >= NumberList.getElementSize() && NumberPart >= NumberOther.NumberList.getElementSize())
                    break;
                intmax_t NumberCurrent = (intmax_t) NumberList[NumberPart] - NumberBorrow;
                if (NumberPart < NumberOther.NumberList.getElementSize()) NumberCurrent -= (intmax_t) NumberOther.NumberList[NumberPart];
                if (NumberCurrent < 0) {
                    NumberBorrow = true;
                    NumberCurrent += 10000000;
                } else NumberBorrow = false;
                NumberResult.addElement(NumberCurrent);
            }
            NumberResult.doRemove0();
            return {NumberResult, true};
        }

        Integer getAbsolute() const noexcept {
            return {NumberList, true};
        }

        Integer getGreatestCommonFactor(const Integer &NumberSource) const noexcept {
            if (isZero()) return NumberSource;
            if (NumberSource.isZero()) return *this;
            if (isEven() && NumberSource.isEven()) return doDivision(2).getGreatestCommonFactor(NumberSource.doDivision(2)).doMultiplication(2);
            else if (isEven()) return doDivision(2).getGreatestCommonFactor(NumberSource);
            else if (NumberSource.isEven()) return getGreatestCommonFactor(NumberSource.doDivision(2));
            return doSubtraction(NumberSource).getAbsolute().getGreatestCommonFactor(Objects::getMinimum(*this, NumberSource));
        }

        Integer getOpposite() const noexcept {
            return {NumberList, !NumberSignature};
        }

        template<Arithmetic T>
        T getValue() const {
            if ((isPositive() && doCompare(::std::numeric_limits<T>::max()) > 0) || (isNegative() && doCompare(::std::numeric_limits<T>::min()) < 0)) [[unlikely]]
                throw ArithmeticException(u"Integer::getValue<T>() (isPositive() && doCompare(std::numeric_limits<T>::max()) > 0) || (isNegative() && doCompare(std::numeric_limits<T>::min()) < 0)"_S);
            T NumberValue(0);
            for (auto NumberPart = NumberList.getElementSize() - 1; NumberPart >= 0; --NumberPart)
                NumberValue = NumberValue * 10000000 + NumberList[NumberPart];
            return NumberSignature ? NumberValue : -NumberValue;
        }

        bool isEven() const noexcept {
            return !(NumberList[0] % 10 & 1);
        }

        bool isNegative() const noexcept {
            return !isZero() && !NumberSignature;
        }

        bool isOdd() const noexcept {
            return NumberList[0] % 10 & 1;
        }

        bool isPositive() const noexcept {
            return !isZero() && NumberSignature;
        }

        bool isZero() const noexcept {
            return NumberList.getElementSize() == 1 && !NumberList.getElementFront();
        }

        String toString() const noexcept override {
            return toString(10);
        }

        String toString(uint8_t NumberRadix) const {
            if (NumberRadix < 2 || NumberRadix > 36) [[unlikely]]
                throw ArithmeticException(u"Integer::toString(uint8_t) NumberRadix"_S);
            StringStream CharacterStream;
            Integer NumberCurrent(getAbsolute()), NumberRadixInteger(NumberRadix);
            if (isZero()) return u"0"_S;
            while (NumberCurrent.NumberList[0]) {
                CharacterStream.addCharacter(Character::valueOf(NumberCurrent.doModulo(NumberRadixInteger).NumberList[0], NumberRadix));
                NumberCurrent.NumberList = NumberCurrent.doDivision(NumberRadixInteger).NumberList;
            }
            if (isNegative()) CharacterStream.addCharacter(u'-');
            return CharacterStream.toString().doReverse();
        }
    };

    template<Arithmetic T>
    class NumberBuiltin final : public Object {
    private:
        T NumberValue;

        friend NumberBuiltin operator%(const NumberBuiltin &Number1, const NumberBuiltin &Number2) {
            return Number1.NumberValue % Number2.NumberValue;
        }

        friend NumberBuiltin operator*(const NumberBuiltin &Number1, const NumberBuiltin &Number2) noexcept {
            return Number1.NumberValue * Number2.NumberValue;
        }

        friend NumberBuiltin operator+(const NumberBuiltin &Number1, const NumberBuiltin &Number2) noexcept {
            return Number1.NumberValue + Number2.NumberValue;
        }

        friend NumberBuiltin operator-(const NumberBuiltin &Number1, const NumberBuiltin &Number2) noexcept {
            return Number1.NumberValue - Number2.NumberValue;
        }

        friend NumberBuiltin operator-(const NumberBuiltin &NumberSource) noexcept {
            return -NumberSource.NumberValue;
        }

        friend NumberBuiltin operator/(const NumberBuiltin &Number1, const NumberBuiltin &Number2) {
            return Number1.NumberValue / Number2.NumberValue;
        }
    public:
        constexpr NumberBuiltin() noexcept = default;

        constexpr NumberBuiltin(T NumberSource) noexcept : NumberValue(NumberSource) {}

        template<Arithmetic O>
        O doCast() const {
            if (Numbers::doCompare(NumberValue, ::std::numeric_limits<O>::max()) > 0 || Numbers::doCompare(NumberValue, ::std::numeric_limits<O>::min()) < 0) throw ArithmeticException(u"IntegerBuiltin::doCast<O(Arithmetic)>() NumberValue"_S);
            return (O) NumberValue;
        }

        intmax_t doCompare(const NumberBuiltin<T> &NumberSource) const noexcept {
            return Numbers::doCompare(NumberValue, NumberSource.NumberValue);
        }

        T getValue() const noexcept {
            return NumberValue;
        }

        uintmax_t hashCode() const noexcept override {
            return NumberValue;
        }

        operator T() const noexcept {
            return NumberValue;
        }

        String toString() const noexcept override {
            return String::valueOf(NumberValue);
        }
    };

    /**
     * Support for fractional operation and storage
     */
    class Fraction final : public Object {
    private:
        Integer NumberDenominator, NumberNumerator;
        bool NumberSignature;

        Fraction(const Integer &NumberNumeratorSource, const Integer &NumberDenominatorSource, bool NumberSignatureSource) noexcept : NumberSignature(NumberSignatureSource) {
            Integer NumberFactor(NumberDenominatorSource.getGreatestCommonFactor(NumberNumeratorSource));
            NumberDenominator = NumberDenominatorSource.doDivision(NumberFactor);
            NumberNumerator = NumberNumeratorSource.doDivision(NumberFactor);
        }

        friend Fraction operator*(const Fraction &Number1, const Fraction &Number2) noexcept {
            return Number1.doMultiplication(Number2);
        }

        friend Fraction operator+(const Fraction &Number1, const Fraction &Number2) noexcept {
            return Number1.doAddition(Number2);
        }

        friend Fraction operator-(const Fraction &Number1, const Fraction &Number2) noexcept {
            return Number1.doSubtraction(Number2);
        }

        friend Fraction operator-(const Fraction &NumberSource) noexcept {
            return NumberSource.getOpposite();
        }

        friend Fraction operator/(const Fraction &Number1, const Fraction &Number2) {
            return Number1.doDivision(Number2);
        }
    public:
        Fraction(const Integer &NumberValueSource) noexcept : NumberDenominator(1), NumberNumerator(NumberValueSource), NumberSignature(NumberValueSource.NumberSignature) {}

        Fraction(const Integer &NumberNumeratorSource, const Integer &NumberDenominatorSource) noexcept {
            NumberSignature = !(NumberDenominatorSource.isPositive() ^ NumberNumeratorSource.isPositive());
            Integer NumberFactor(NumberDenominatorSource.getGreatestCommonFactor(NumberNumeratorSource));
            NumberDenominator = NumberDenominatorSource.getAbsolute().doDivision(NumberFactor);
            NumberNumerator = NumberNumeratorSource.getAbsolute().doDivision(NumberFactor);
        }

        Fraction doAddition(const Fraction &NumberOther) const noexcept {
            if (isPositive() && NumberOther.isNegative()) return doSubtraction(NumberOther.getAbsolute());
            if (isNegative() && NumberOther.isPositive()) return NumberOther.doSubtraction(getAbsolute());
            if (isNegative() && NumberOther.isNegative()) return getAbsolute().doAddition(NumberOther.getAbsolute()).getOpposite();
            return {NumberNumerator.doMultiplication(NumberOther.NumberDenominator).doAddition(NumberDenominator.doMultiplication(NumberOther.NumberNumerator)), NumberDenominator.doMultiplication(NumberOther.NumberDenominator)};
        }

        intmax_t doCompare(const Fraction &NumberOther) const noexcept {
            if (NumberSignature != NumberOther.NumberSignature && NumberNumerator.doCompare(0) && NumberDenominator.doCompare(0)) return Numbers::doCompare(NumberSignature, NumberOther.NumberSignature);
            return NumberNumerator.doMultiplication(NumberOther.NumberDenominator).doCompare(NumberDenominator.doMultiplication(NumberOther.NumberNumerator));
        }

        Fraction doDivision(const Fraction &NumberOther) const noexcept {
            return {NumberNumerator.doMultiplication(NumberOther.NumberDenominator), NumberDenominator.doMultiplication(NumberOther.NumberNumerator), !(NumberSignature ^ NumberOther.NumberSignature)};
        }

        Fraction doMultiplication(const Fraction &NumberOther) const noexcept {
            return {NumberNumerator.doMultiplication(NumberOther.NumberNumerator), NumberDenominator.doMultiplication(NumberOther.NumberDenominator), !(NumberSignature ^ NumberOther.NumberSignature)};
        }

        Fraction doMultiplication(const Integer &NumberOther) const noexcept {
            return {NumberNumerator.doMultiplication(NumberOther.getAbsolute()), NumberDenominator, !(NumberSignature ^ NumberOther.NumberSignature)};
        }

        Fraction doPower(const Integer &NumberExponent) const noexcept {
            if (NumberExponent.doCompare(0) < 0) return {NumberDenominator.doPower(NumberExponent.getAbsolute()), NumberNumerator.doPower(NumberExponent.getAbsolute())};
            return {NumberNumerator.doPower(NumberExponent), NumberDenominator.doPower(NumberExponent)};
        }

        Fraction doSubtraction(const Fraction &NumberOther) const noexcept {
            if (NumberOther.NumberNumerator.isZero()) return *this;
            if (isPositive() && NumberOther.isNegative()) return doAddition(NumberOther.getAbsolute());
            if (doCompare(NumberOther) < 0) return NumberOther.doSubtraction(*this).getOpposite();
            return {NumberNumerator.doMultiplication(NumberOther.NumberDenominator).doSubtraction(NumberDenominator.doMultiplication(NumberOther.NumberNumerator)), NumberDenominator.doMultiplication(NumberOther.NumberDenominator)};
        }

        Fraction getAbsolute() const noexcept {
            return {NumberNumerator, NumberDenominator, true};
        }

        Integer getDenominator() const noexcept {
            return NumberDenominator;
        }

        Integer getNumerator() const noexcept {
            return NumberNumerator;
        }

        Fraction getOpposite() const noexcept {
            return {NumberNumerator, NumberDenominator, !NumberSignature};
        }

        template<typename T>
        [[deprecated]] auto getValue() const noexcept {
            return (NumberSignature ? 1.0 : -1.0) * NumberNumerator.getValue<T>() / NumberDenominator.getValue<T>();
        }

        bool isNegative() const noexcept {
            return !NumberSignature && !NumberNumerator.isZero();
        }

        bool isPositive() const noexcept {
            return NumberSignature && !NumberNumerator.isZero();
        }

        Integer toInteger() const noexcept {
            return NumberNumerator.doDivision(NumberDenominator);
        }

        String toString() const noexcept override {
            StringStream CharacterStream;
            if (NumberNumerator.isZero()) return u"0"_S;
            if (!NumberSignature) CharacterStream.addCharacter(u'-');
            CharacterStream.addString(NumberNumerator.toString());
            CharacterStream.addCharacter(u'/');
            CharacterStream.addString(NumberDenominator.toString());
            return CharacterStream.toString();
        }
    };

    inline namespace Literal {
        Integer operator"" _I(const char16_t *StringSource, size_t StringSize) {
            return {{::std::u16string{StringSource, StringSize}}, 10};
        }

        Integer operator""_I(unsigned long long int NumberValue) noexcept {
            return {NumberValue};
        }
    }
}

#endif
