#pragma once

#include <Core/Exception.hpp>

#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace eLibrary {
    class Integer final : public Object {
    private:
        bool NumberSignature;
        std::vector<intmax_t> NumberList;
    public:
        Integer(intmax_t NumberValue = 0) noexcept: NumberSignature(NumberValue >= 0) {
            NumberValue = NumberValue >= 0? NumberValue : -NumberValue;
            NumberList.clear();
            do {
                NumberList.push_back(NumberValue % 100000);
                NumberValue /= 100000;
            } while (NumberValue > 0);
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
                else throw Exception(String(u"Integer::Integer(String*, unsigned short) NumberValue"));
            NumberList.clear();
            for (; NumberDigit < NumberValue.getCharacterSize(); ++NumberDigit)
                operator=(doMultiplication(IntegerRadix).doAddition(NumberDigitMapping[NumberValue.getCharacter(NumberDigit)]));
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
                if (!NumberCarry && NumberPart >= NumberList.size() &&
                    NumberPart >= NumberOther.NumberList.size())
                    break;
                intmax_t NumberCurrent = NumberCarry;
                if (NumberPart < NumberList.size()) NumberCurrent += NumberList[NumberPart];
                if (NumberPart < NumberOther.NumberList.size()) NumberCurrent += NumberOther.NumberList[NumberPart];
                NumberResult.NumberList.push_back(NumberCurrent % 100000);
                NumberCarry = NumberCurrent / 100000;
            }
            return NumberResult;
        }

        intmax_t doCompare(const Integer &NumberOther) const noexcept {
            if (NumberSignature != NumberOther.NumberSignature)
                return (intmax_t)NumberSignature - NumberOther.NumberSignature;
            if (NumberList.size() != NumberOther.NumberList.size())
                return (intmax_t) (NumberList.size()) - NumberOther.NumberList.size();
            for (auto NumberPart = (intmax_t) (NumberList.size() - 1); NumberPart >= 0; --NumberPart)
                if (NumberList[NumberPart] != NumberOther.NumberList[NumberPart])
                    return NumberSignature ? (intmax_t) (NumberList[NumberPart]) - NumberOther.NumberList[NumberPart] :
                           (intmax_t) (NumberOther.NumberList[NumberPart]) - NumberList[NumberPart];
            return 0;
        }

        Integer doDivision(const Integer &NumberOther) const noexcept {
            Integer NumberRemainder, NumberResult(*this);
            if (NumberSignature ^ NumberOther.NumberSignature) NumberResult.NumberSignature = false;
            for (auto NumberPart = (intmax_t) (NumberList.size() - 1); NumberPart >= 0; --NumberPart) {
                NumberRemainder = NumberRemainder.doMultiplication(100000).doAddition(NumberList[NumberPart]);
                intmax_t NumberMiddle, NumberStart = 0, NumberStop = 99999999LL;
                for (;;) {
                    NumberMiddle = (NumberStart + NumberStop) >> 1;
                    if (NumberOther.doMultiplication(NumberMiddle).doCompare(NumberRemainder) <= 0)
                        if (NumberOther.doMultiplication(NumberMiddle + 1).doCompare(NumberRemainder) > 0) break;
                        else NumberStart = NumberMiddle;
                    else NumberStop = NumberMiddle;
                }
                NumberResult.NumberList[NumberPart] = NumberMiddle;
                NumberRemainder = NumberRemainder.doSubtraction(NumberOther.doMultiplication(NumberResult.NumberList[NumberPart]));
            }
            while (!NumberResult.NumberList.back() && NumberResult.NumberList.size() > 1)
                NumberResult.NumberList.pop_back();
            return NumberResult;
        }

        Integer doModulo(const Integer &NumberOther) const noexcept {
            Integer NumberRemainder, NumberResult(*this);
            if (NumberSignature ^ NumberOther.NumberSignature) NumberResult.NumberSignature = false;
            for (auto NumberPart = (intmax_t) (NumberList.size() - 1); NumberPart >= 0; --NumberPart) {
                NumberRemainder = NumberRemainder.doMultiplication(100000).doAddition(NumberList[NumberPart]);
                intmax_t NumberMiddle, NumberStart = 0, NumberStop = 100000 - 1;
                for (;;) {
                    NumberMiddle = (NumberStart + NumberStop) >> 1;
                    if (NumberOther.doMultiplication(NumberMiddle).doCompare(NumberRemainder) <= 0)
                        if (NumberOther.doMultiplication(NumberMiddle + 1).doCompare(NumberRemainder) > 0) break;
                        else NumberStart = NumberMiddle;
                    else NumberStop = NumberMiddle;
                }
                NumberResult.NumberList[NumberPart] = NumberMiddle;
                Integer NumberRemainderNew = NumberRemainder.doSubtraction(NumberOther.doMultiplication(NumberResult.NumberList[NumberPart]));
                NumberRemainder.NumberList = NumberRemainderNew.NumberList;
            }
            return NumberRemainder;
        }

        Integer doMultiplication(const Integer &NumberOther) const noexcept {
            std::vector<intmax_t> NumberProduct(NumberList.size() + NumberOther.NumberList.size(), 0);
            for (size_t NumberDigit1 = 0; NumberDigit1 < NumberList.size(); ++NumberDigit1)
                for (size_t NumberDigit2 = 0; NumberDigit2 < NumberOther.NumberList.size(); ++NumberDigit2)
                    NumberProduct[NumberDigit1 + NumberDigit2] +=
                            NumberList[NumberDigit1] * NumberOther.NumberList[NumberDigit2];
            Integer NumberResult;
            NumberResult.NumberList.clear();
            if (NumberSignature ^ NumberOther.NumberSignature) NumberResult.NumberSignature = false;
            intmax_t NumberCarry = 0;
            for (size_t NumberPart = 0;; ++NumberPart) {
                if (!NumberCarry && NumberPart >= NumberProduct.size()) break;
                intmax_t NumberCurrent = NumberProduct[NumberPart] + NumberCarry;
                NumberResult.NumberList.push_back(NumberCurrent % 100000);
                NumberCarry = NumberCurrent / 100000;
            }
            while (!NumberResult.NumberList.back() && NumberResult.NumberList.size() > 1)
                NumberResult.NumberList.pop_back();
            return NumberResult;
        }

        Integer doSubtraction(const Integer &NumberOther) const noexcept {
            Integer NumberResult;
            NumberResult.NumberList.clear();
            intmax_t NumberBorrow = 0;
            for (size_t NumberPart = 0;; ++NumberPart) {
                if (!NumberBorrow && NumberPart >= NumberList.size() &&
                    NumberPart >= NumberOther.NumberList.size())
                    break;
                intmax_t NumberCurrent = NumberList[NumberPart] - NumberBorrow;
                if (NumberPart < NumberOther.NumberList.size()) NumberCurrent -= NumberOther.NumberList[NumberPart];
                if (NumberCurrent < 0) {
                    NumberBorrow = 1;
                    NumberCurrent += 100000;
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

        intmax_t getValue() const noexcept {
            intmax_t NumberValue = NumberList.back();
            if (NumberList.size() == 1) return NumberValue;
            for (auto NumberPart = (intmax_t) NumberList.size() - 2; NumberPart >= 0; --NumberPart)
                NumberValue = NumberValue * 100000 + NumberList[NumberPart];
            return NumberValue;
        }

        Integer getOpposite() const noexcept {
            Integer NumberResult(*this);
            NumberResult.NumberSignature = !NumberSignature;
            return NumberResult;
        }

        String toString() const noexcept {
            std::stringstream StringStream;
            if (!NumberSignature) StringStream << '-';
            StringStream << NumberList.back();
            if (NumberList.size() == 1) return String(StringStream.str());
            for (auto NumberPart = (intmax_t) (NumberList.size() - 2); NumberPart >= 0; --NumberPart)
                StringStream << std::setw(10) << std::setfill('0') << NumberList[NumberPart];
            return String(StringStream.str());
        }
    };

    class Fraction final {
    private:
        Integer NumberDenominator, NumberNumerator;
    public:
        Fraction(const Integer &Numerator, const Integer &Denominator) noexcept : NumberDenominator(Denominator), NumberNumerator(Numerator) {}

        Fraction doAddition(const Fraction &NumberOther) const noexcept {
            return Fraction(NumberNumerator.doMultiplication(NumberOther.NumberDenominator).doAddition(NumberDenominator.doMultiplication(NumberOther.NumberNumerator)), NumberDenominator.doMultiplication(NumberOther.NumberDenominator));
        }
    };
}
