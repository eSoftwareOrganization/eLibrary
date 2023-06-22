#pragma once

#include <Core/Exception.hpp>

#include <algorithm>
#include <iomanip>
#include <limits>
#include <map>
#include <sstream>
#include <vector>

namespace eLibrary::Core {
#define NumberBaseUnit 10000000

    /**
     * The Integer class provides support for integral operation and storage
     */
    class Integer final : public Object {
    private:
        bool NumberSignature;
        std::vector<uintmax_t> NumberList;

        template<std::signed_integral T>
        static T getAbsolute(T NumberSource) noexcept {
            return NumberSource >= 0 ? NumberSource : -NumberSource;
        }

        template<std::unsigned_integral T>
        static T getAbsolute(T NumberSource) noexcept {
            return NumberSource;
        }
    public:
        constexpr Integer() noexcept : NumberSignature(true) {
            NumberList.push_back(0);
            static_assert(std::is_integral<decltype(NumberBaseUnit)>::value && NumberBaseUnit > 0, "Invalid NumberBaseUnit");
        }

        template<std::integral T>
        Integer(T NumberValue) noexcept : NumberSignature(NumberValue >= 0) {
            do {
                NumberList.push_back(getAbsolute(NumberValue % NumberBaseUnit));
                NumberValue /= NumberBaseUnit;
            } while (NumberValue);
        }

        Integer(const String &NumberValue, unsigned short NumberRadix = 10) : NumberSignature(true) {
            if (NumberRadix < 2 || NumberRadix > 36)
                throw Exception(String(u"Integer::Integer(const String&, unsigned short) NumberRadix"));
            if (NumberValue.isEmpty())
                throw Exception(String(u"Integer::Integer(const String&, unsigned short) NumberValue"));
            Integer IntegerRadix(NumberRadix);
            bool NumberSignatureExist = false;
            static std::map<char16_t, unsigned short> NumberDigitMapping;
            if (NumberDigitMapping.empty()) {
                for (unsigned short NumberDigit = 0; NumberDigit < 10; ++NumberDigit)
                    NumberDigitMapping[NumberDigit + 48] = NumberDigit;
                for (unsigned short NumberDigit = 0; NumberDigit < 26; ++NumberDigit)
                    NumberDigitMapping[NumberDigit + 65] = NumberDigitMapping[NumberDigit + 97] = NumberDigit + 10;
            }
            intmax_t NumberDigit = 0;
            for (; NumberDigit < NumberValue.getCharacterSize(); ++NumberDigit)
                if (NumberValue.getCharacter(NumberDigit) == u'-' && NumberDigit == 0 && !NumberSignatureExist)
                    NumberSignature = false, NumberSignatureExist = true;
                else if (NumberValue.getCharacter(NumberDigit) == u'+' && NumberDigit == 0 && !NumberSignatureExist)
                    NumberSignatureExist = true;
                else if ((iswdigit(NumberValue.getCharacter(NumberDigit)) || iswalpha(NumberValue.getCharacter(NumberDigit))) && NumberDigitMapping[NumberValue.getCharacter(NumberDigit)] < NumberRadix);
                else throw Exception(String(u"Integer::Integer(const String&, unsigned short) NumberValue"));
            for (NumberDigit = 0; NumberDigit < NumberValue.getCharacterSize(); ++NumberDigit)
                NumberList = doMultiplication(IntegerRadix).doAddition(NumberDigitMapping[NumberValue.getCharacter(NumberDigit)]).NumberList;
            if (NumberList.empty()) NumberList.push_back(0);
            while (!NumberList.back() && NumberList.size() > 1) NumberList.pop_back();
        }

        Integer doAddition(const Integer &NumberOther) const noexcept {
            if (NumberSignature && !NumberOther.NumberSignature) return doSubtraction(NumberOther.getAbsolute());
            if (!NumberSignature && NumberOther.NumberSignature) return NumberOther.doSubtraction(getAbsolute());
            if (!NumberSignature && !NumberOther.NumberSignature) return getAbsolute().doAddition(NumberOther.getAbsolute()).getOpposite();
            Integer NumberResult;
            NumberResult.NumberList.clear();
            intmax_t NumberCarry = 0;
            for (size_t NumberPart = 0;; ++NumberPart) {
                if (!NumberCarry && NumberPart >= NumberList.size() && NumberPart >= NumberOther.NumberList.size())
                    break;
                intmax_t NumberCurrent = NumberCarry;
                if (NumberPart < NumberList.size()) NumberCurrent += NumberList[NumberPart];
                if (NumberPart < NumberOther.NumberList.size()) NumberCurrent += NumberOther.NumberList[NumberPart];
                NumberResult.NumberList.push_back(NumberCurrent % NumberBaseUnit);
                NumberCarry = NumberCurrent / NumberBaseUnit;
            }
            return NumberResult;
        }

        intmax_t doCompare(const Integer &NumberOther) const noexcept {
            if (NumberSignature != NumberOther.NumberSignature && (NumberList.size() != 1 || NumberList[0]) && (NumberOther.NumberList.size() != 1 || NumberList[0]))
                return (intmax_t) NumberSignature - NumberOther.NumberSignature;
            if (NumberList.size() != NumberOther.NumberList.size())
                return NumberSignature ? (intmax_t) NumberList.size() - NumberOther.NumberList.size() : (intmax_t) NumberOther.NumberList.size() - NumberList.size();
            for (auto NumberPart = (intmax_t) (NumberList.size() - 1); NumberPart >= 0; --NumberPart)
                if (NumberList[NumberPart] != NumberOther.NumberList[NumberPart])
                    return NumberSignature ? (intmax_t) (NumberList[NumberPart]) - NumberOther.NumberList[NumberPart] :
                           (intmax_t) (NumberOther.NumberList[NumberPart]) - NumberList[NumberPart];
            return 0;
        }

        Integer doDivision(const Integer &NumberOther) const {
            if (!NumberOther.doCompare(0)) throw ArithmeticException(String(u"Integer::doDivision(const Integer&) Divide By 0"));
            Integer NumberRemainder, NumberResult(*this);
            NumberResult.NumberSignature = !(NumberSignature ^ NumberOther.NumberSignature);
            for (auto NumberPart = (intmax_t) (NumberList.size() - 1); NumberPart >= 0; --NumberPart) {
                NumberRemainder = NumberRemainder.doMultiplication(NumberBaseUnit).doAddition(NumberList[NumberPart]);
                intmax_t NumberMiddle, NumberStart = 0, NumberStop = NumberBaseUnit - 1;
                for (;;) {
                    NumberMiddle = (NumberStart + NumberStop) >> 1;
                    if (NumberOther.getAbsolute().doMultiplication(NumberMiddle).doCompare(NumberRemainder) <= 0)
                        if (NumberOther.getAbsolute().doMultiplication(NumberMiddle + 1).doCompare(NumberRemainder) > 0) break;
                        else NumberStart = NumberMiddle;
                    else NumberStop = NumberMiddle;
                }
                NumberResult.NumberList[NumberPart] = NumberMiddle;
                NumberRemainder = NumberRemainder.doSubtraction(NumberOther.getAbsolute().doMultiplication(NumberResult.NumberList[NumberPart]));
            }
            while (!NumberResult.NumberList.back() && NumberResult.NumberList.size() > 1)
                NumberResult.NumberList.pop_back();
            return NumberResult;
        }

        Integer doFactorial() const noexcept {
            Integer NumberCurrent(*this), NumberResult(1);
            if (doCompare(1) <= 0) return NumberResult;
            while (NumberCurrent.doCompare(1)) {
                NumberResult = NumberResult.doMultiplication(NumberCurrent);
                NumberCurrent = NumberCurrent.doSubtraction(1);
            }
            return NumberResult;
        }

        Integer doModulo(const Integer &NumberOther) const {
            if (!NumberOther.doCompare(0)) throw ArithmeticException(String(u"Integer::doModulo(const Integer&) Modulo By 0"));
            Integer NumberRemainder, NumberResult(*this);
            for (auto NumberPart = (intmax_t) (NumberList.size() - 1); NumberPart >= 0; --NumberPart) {
                NumberRemainder = NumberRemainder.doMultiplication(NumberBaseUnit).doAddition(NumberList[NumberPart]);
                intmax_t NumberMiddle, NumberStart = 0, NumberStop = NumberBaseUnit - 1;
                for (;;) {
                    NumberMiddle = (NumberStart + NumberStop) >> 1;
                    if (NumberOther.getAbsolute().doMultiplication(NumberMiddle).doCompare(NumberRemainder) <= 0)
                        if (NumberOther.getAbsolute().doMultiplication(NumberMiddle + 1).doCompare(NumberRemainder) > 0) break;
                        else NumberStart = NumberMiddle;
                    else NumberStop = NumberMiddle;
                }
                NumberResult.NumberList[NumberPart] = NumberMiddle;
                NumberRemainder = NumberRemainder.doSubtraction(NumberOther.getAbsolute().doMultiplication(NumberResult.NumberList[NumberPart]));
            }
            NumberRemainder.NumberSignature = NumberSignature;
            return NumberRemainder;
        }

        Integer doMultiplication(const Integer &NumberOther) const noexcept {
            std::vector<intmax_t> NumberProduct(NumberList.size() + NumberOther.NumberList.size(), 0);
            for (size_t NumberDigit1 = 0; NumberDigit1 < NumberList.size(); ++NumberDigit1)
                for (size_t NumberDigit2 = 0; NumberDigit2 < NumberOther.NumberList.size(); ++NumberDigit2)
                    NumberProduct[NumberDigit1 + NumberDigit2] += NumberList[NumberDigit1] * NumberOther.NumberList[NumberDigit2];
            Integer NumberResult;
            NumberResult.NumberList.clear();
            NumberResult.NumberSignature = !(NumberSignature ^ NumberOther.NumberSignature);
            intmax_t NumberCarry = 0;
            for (size_t NumberPart = 0;; ++NumberPart) {
                if (!NumberCarry && NumberPart >= NumberList.size() + NumberOther.NumberList.size()) break;
                intmax_t NumberCurrent = NumberProduct[NumberPart] + NumberCarry;
                NumberResult.NumberList.push_back(NumberCurrent % NumberBaseUnit);
                NumberCarry = NumberCurrent / NumberBaseUnit;
            }
            while (!NumberResult.NumberList.back() && NumberResult.NumberList.size() > 1)
                NumberResult.NumberList.pop_back();
            return NumberResult;
        }

        Integer doPower(const Integer &NumberExponentSource) const {
            if (!doCompare(0) && !NumberExponentSource.doCompare(0)) throw ArithmeticException({u"Integer::doPower(const Integer&, const Integer&) 0⁰"});
            if (NumberExponentSource.isNegative()) throw ArithmeticException(String(u"Integer::doPower(const Integer&) NumberExponentSource"));
            Integer NumberBase(*this), NumberExponent(NumberExponentSource), NumberResult(1);
            while (NumberExponent.doCompare(0)) {
                if (NumberExponent.isOdd()) NumberResult = NumberResult.doMultiplication(NumberBase);
                NumberBase = NumberBase.doMultiplication(NumberBase);
                NumberExponent = NumberExponent.doDivision(2);
            }
            return NumberResult;
        }

        Integer doPower(const Integer &NumberExponentSource, const Integer &NumberModulo) const {
            if (!doCompare(0) && !NumberExponentSource.doCompare(0)) throw ArithmeticException({u"Integer::doPower(const Integer&, const Integer&) 0⁰"});
            if (NumberExponentSource.isNegative()) throw ArithmeticException(String(u"Integer::doPower(const Integer&, const Integer&) NumberExponentSource"));
            Integer NumberBase(*this), NumberExponent(NumberExponentSource), NumberResult(1);
            while (NumberExponent.doCompare(0)) {
                if (NumberExponent.isOdd()) NumberResult = NumberResult.doMultiplication(NumberBase).doModulo(NumberModulo);
                NumberBase = NumberBase.doMultiplication(NumberBase).doModulo(NumberModulo);
                NumberExponent = NumberExponent.doDivision(2);
            }
            return NumberResult;
        }

        Integer doSubtraction(const Integer &NumberOther) const noexcept {
            if (NumberOther.isNegative()) return doAddition(NumberOther.getAbsolute());
            if (doCompare(NumberOther) < 0) return NumberOther.doSubtraction(*this).getOpposite();
            Integer NumberResult;
            NumberResult.NumberList.clear();
            intmax_t NumberBorrow = 0;
            for (uintmax_t NumberPart = 0;; ++NumberPart) {
                if (!NumberBorrow && NumberPart >= NumberList.size() && NumberPart >= NumberOther.NumberList.size())
                    break;
                intmax_t NumberCurrent = (intmax_t) NumberList[NumberPart] - NumberBorrow;
                if (NumberPart < NumberOther.NumberList.size()) NumberCurrent -= (intmax_t) NumberOther.NumberList[NumberPart];
                if (NumberCurrent < 0) {
                    NumberBorrow = 1;
                    NumberCurrent += NumberBaseUnit;
                } else NumberBorrow = 0;
                NumberResult.NumberList.push_back(NumberCurrent);
            }
            while (!NumberResult.NumberList.back() && NumberResult.NumberList.size() > 1)
                NumberResult.NumberList.pop_back();
            return NumberResult;
        }

        Integer getAbsolute() const noexcept {
            Integer NumberResult(*this);
            NumberResult.NumberSignature = true;
            return NumberResult;
        }

        Integer getOpposite() const noexcept {
            Integer NumberResult(*this);
            NumberResult.NumberSignature = !NumberSignature;
            return NumberResult;
        }

        template<std::integral T>
        T getValue() const {
            if ((isPositive() && doCompare(std::numeric_limits<T>::max()) > 0) || (isNegative() && doCompare(std::numeric_limits<T>::min()) < 0))
                throw ArithmeticException(String(u"Integer::getValue<T>() (isPositive() && doCompare(std::numeric_limits<T>::max()) > 0) || (isNegative() && doCompare(std::numeric_limits<T>::min()) < 0)"));
            T NumberValue = NumberList.back();
            if (NumberList.size() == 1) return NumberSignature ? NumberValue : -NumberValue;
            for (auto NumberPart = (intmax_t) NumberList.size() - 2; NumberPart >= 0; --NumberPart)
                NumberValue = NumberValue * NumberBaseUnit + NumberList[NumberPart];
            return NumberSignature ? NumberValue : -NumberValue;
        }

        bool isEven() const noexcept {
            return !(NumberList[0] % 10 & 1);
        }

        bool isNegative() const noexcept {
            return doCompare(0) && !NumberSignature;
        }

        bool isOdd() const noexcept {
            return NumberList[0] % 10 & 1;
        }

        bool isPositive() const noexcept {
            return doCompare(0) && NumberSignature;
        }

        String toString() const noexcept override {
            return toString(10);
        }

        String toString(unsigned short NumberRadix) const {
            if (NumberRadix < 2 || NumberRadix > 36) throw Exception(String(u"Integer::toString(unsigned short) NumberRadix"));
            StringStream CharacterStream;
            static std::map<unsigned short, char16_t> NumberDigitMapping;
            if (NumberDigitMapping.empty()) {
                for (unsigned short NumberDigit = 0; NumberDigit < 10; ++NumberDigit)
                    NumberDigitMapping[NumberDigit] = char16_t(NumberDigit + 48);
                for (unsigned short NumberDigit = 0; NumberDigit < 26; ++NumberDigit)
                    NumberDigitMapping[NumberDigit + 10] = char16_t(NumberDigit + 65);
            }
            Integer NumberCurrent(getAbsolute()), NumberRadixInteger(NumberRadix);
            if (!doCompare(0)) return {u"0"};
            while (NumberCurrent.NumberList[0]) {
                CharacterStream.addCharacter(NumberDigitMapping[NumberCurrent.doModulo(NumberRadixInteger).NumberList[0]]);
                NumberCurrent.NumberList = NumberCurrent.doDivision(NumberRadixInteger).NumberList;
            }
            if (!NumberSignature) CharacterStream.addCharacter(u'-');
            return CharacterStream.toString().doReverse();
        }
    };

    /**
     * The Fraction class provides support for fractional operation and storage
     */
    class Fraction final : public Object {
    private:
        bool NumberSignature;
        Integer NumberDenominator, NumberNumerator;

        Integer getGreatestCommonFactor(const Integer &Number1, const Integer &Number2) const noexcept {
            if (!Number2.getAbsolute().doCompare(Integer(0))) return Number1;
            return getGreatestCommonFactor(Number2, Number1.doModulo(Number2));
        }
    public:
        Fraction(const Integer &NumberValueSource) noexcept : NumberSignature(NumberValueSource.isPositive()), NumberDenominator(1), NumberNumerator(NumberValueSource) {}

        Fraction(const Integer &NumberNumeratorSource, const Integer &NumberDenominatorSource) noexcept {
            NumberSignature = !(NumberDenominatorSource.isPositive() ^ NumberNumeratorSource.isPositive());
            Integer NumberFactor = getGreatestCommonFactor(NumberDenominatorSource, NumberNumeratorSource);
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
            if (NumberSignature != NumberOther.NumberSignature && NumberNumerator.doCompare(Integer(0)) && NumberDenominator.doCompare(Integer(0))) return (intmax_t) NumberSignature - NumberOther.NumberSignature;
            return NumberNumerator.doMultiplication(NumberOther.NumberDenominator).doCompare(NumberDenominator.doMultiplication(NumberOther.NumberNumerator));
        }

        Fraction doDivision(const Fraction &NumberOther) const noexcept {
            Fraction NumberResult(NumberNumerator.doMultiplication(NumberOther.NumberDenominator), NumberDenominator.doMultiplication(NumberOther.NumberNumerator));
            NumberResult.NumberSignature = !(NumberSignature ^ NumberOther.NumberSignature);
            return NumberResult;
        }

        Fraction doMultiplication(const Fraction &NumberOther) const noexcept {
            Fraction NumberResult(NumberNumerator.doMultiplication(NumberOther.NumberNumerator), NumberDenominator.doMultiplication(NumberOther.NumberDenominator));
            NumberResult.NumberSignature = !(NumberSignature ^ NumberOther.NumberSignature);
            return NumberResult;
        }

        Fraction doMultiplication(const Integer &NumberOther) const noexcept {
            Fraction NumberResult(NumberNumerator.doMultiplication(NumberOther.getAbsolute()), NumberDenominator);
            NumberResult.NumberSignature = !(NumberSignature ^ NumberOther.isPositive());
            return NumberResult;
        }

        Fraction doPower(const Integer &NumberExponent) const noexcept {
            if (NumberExponent.doCompare(0) < 0) return {NumberDenominator.doPower(NumberExponent.getAbsolute()), NumberNumerator.doPower(NumberExponent.getAbsolute())};
            return {NumberNumerator.doPower(NumberExponent), NumberDenominator.doPower(NumberExponent)};
        }

        Fraction doSubtraction(const Fraction &NumberOther) const noexcept {
            if (!NumberOther.NumberNumerator.doCompare(0)) return *this;
            if (isPositive() && NumberOther.isNegative()) return doAddition(NumberOther.getAbsolute());
            if (doCompare(NumberOther) < 0) return NumberOther.doSubtraction(*this).getOpposite();
            return {NumberNumerator.doMultiplication(NumberOther.NumberDenominator).doSubtraction(NumberDenominator.doMultiplication(NumberOther.NumberNumerator)), NumberDenominator.doMultiplication(NumberOther.NumberDenominator)};
        }

        Fraction getAbsolute() const noexcept {
            return {NumberNumerator, NumberDenominator};
        }

        Integer getDenominator() const noexcept {
            return NumberDenominator;
        }

        Integer getNumerator() const noexcept {
            return NumberNumerator;
        }

        Fraction getOpposite() const noexcept {
            Fraction NumberResult(NumberNumerator, NumberDenominator);
            NumberResult.NumberSignature = !NumberSignature;
            return NumberResult;
        }

        template<typename T>
        [[deprecated]] auto getValue() const noexcept {
            return (NumberSignature ? 1.0 : -1.0) * NumberNumerator.getValue<T>() / NumberDenominator.getValue<T>();
        }

        bool isNegative() const noexcept {
            return !NumberSignature && NumberNumerator.doCompare(0);
        }

        bool isPositive() const noexcept {
            return NumberSignature && NumberNumerator.doCompare(0);
        }

        String toString() const noexcept override {
            StringStream CharacterStream;
            if (!NumberNumerator.doCompare(0)) return {u"0"};
            if (!NumberSignature) CharacterStream.addCharacter(u'-');
            CharacterStream.addString(NumberNumerator.toString());
            CharacterStream.addCharacter(u'/');
            CharacterStream.addString(NumberDenominator.toString());
            return CharacterStream.toString();
        }
    };
}
