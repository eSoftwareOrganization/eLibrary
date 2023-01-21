#pragma once

#include <algorithm>
#include <codecvt>
#include <cuchar>
#include <locale>
#include <map>
#include <queue>
#include <sstream>
#include <string>

#include <Core/Object.hpp>

namespace eLibrary {
    class String final : public Object {
    private:
        intmax_t CharacterSize;
        char16_t *CharacterContainer;
        uintmax_t *CharacterReference;
    public:
        String() noexcept: CharacterSize(0), CharacterContainer(nullptr) {
            CharacterReference = new uintmax_t;
            *CharacterReference = 1;
        }

        String(const String &StringSource) noexcept : CharacterSize(StringSource.CharacterSize), CharacterContainer(StringSource.CharacterContainer), CharacterReference(StringSource.CharacterReference) {
            ++(*CharacterReference);
        }

        String(const std::string &StringSource) noexcept : CharacterSize((intmax_t) StringSource.size()) {
            static std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> StringConverter;
            std::u16string String16Source = StringConverter.from_bytes(StringSource);
            CharacterContainer = new char16_t[String16Source.size() + 1];
            std::copy(String16Source.begin(), String16Source.end(), CharacterContainer);
            CharacterContainer[CharacterSize] = char16_t();
            CharacterReference = new uintmax_t;
            *CharacterReference = 1;
        }

        String(const std::u16string &StringSource) noexcept : CharacterSize((intmax_t) StringSource.size()) {
            CharacterContainer = new char16_t[StringSource.size() + 1];
            std::copy(StringSource.begin(), StringSource.end(), CharacterContainer);
            CharacterContainer[CharacterSize] = char16_t();
            CharacterReference = new uintmax_t;
            *CharacterReference = 1;
        }

        String(const std::u32string &StringSource) noexcept : CharacterSize((intmax_t) StringSource.size()) {
            static std::wstring_convert<std::codecvt_utf16<char32_t>, char32_t> StringConverter;
            std::string String8Source = StringConverter.to_bytes(StringSource);
            std::u16string String16Source(reinterpret_cast<const char16_t*>(String8Source.c_str()), String8Source.size() / sizeof(char16_t));
            CharacterContainer = new char16_t[String16Source.size() + 1];
            std::copy(String16Source.begin(), String16Source.end(), CharacterContainer);
            CharacterContainer[CharacterSize] = char16_t();
            CharacterReference = new uintmax_t;
            *CharacterReference = 1;
        }

        String(const std::wstring &StringSource) noexcept : CharacterSize((intmax_t) StringSource.size()) {
            CharacterContainer = new char16_t[StringSource.size() + 1];
            std::copy(StringSource.begin(), StringSource.end(), CharacterContainer);
            CharacterContainer[CharacterSize] = char16_t();
            CharacterReference = new uintmax_t;
            *CharacterReference = 1;
        }

        ~String() noexcept {
            if (--(*CharacterReference) == 0) {
                CharacterSize = 0;
                delete[] CharacterContainer;
                CharacterContainer = nullptr;
                delete[] CharacterReference;
                CharacterReference = nullptr;
            }
        }

        void doAssign(const String &StringSource) noexcept {
            if (&StringSource == this) return;
            delete[] CharacterContainer;
            delete[] CharacterReference;
            CharacterContainer = StringSource.CharacterContainer;
            CharacterSize = StringSource.CharacterSize;
            CharacterReference = StringSource.CharacterReference;
            ++(*CharacterReference);
        }

        intmax_t doCompare(const String &StringOther) const noexcept {
            if (CharacterSize != StringOther.CharacterSize) return CharacterSize - StringOther.CharacterSize;
            for (intmax_t CharacterIndex = 0; CharacterIndex < CharacterSize; ++CharacterIndex)
                if (CharacterContainer[CharacterIndex] != StringOther.CharacterContainer[CharacterIndex])
                    return CharacterContainer[CharacterIndex] - StringOther.CharacterContainer[CharacterIndex];
            return 0;
        }

        String doConcat(char16_t CharacterSource) const noexcept {
            std::basic_stringstream<char16_t> CharacterStream;
            CharacterStream << CharacterContainer << CharacterSource;
            return CharacterStream.str();
        }

        String doConcat(const String &StringOther) const noexcept {
            std::basic_stringstream<char16_t> CharacterStream;
            CharacterStream << CharacterContainer << StringOther.CharacterContainer;
            return CharacterStream.str();
        }

        intmax_t doFind(char16_t CharacterSource) const noexcept {
            for (char16_t CharacterIndex = 0;CharacterIndex < CharacterSize;++CharacterIndex)
                if (CharacterContainer[CharacterIndex] == CharacterSource) return CharacterIndex;
            return -1;
        }

        intmax_t doFind(const String &StringTarget) const noexcept {
            intmax_t Character1, Character2;
            for (Character1 = 0, Character2 = 0;Character1 < CharacterSize && Character2 < StringTarget.CharacterSize; ++Character1)
                if (CharacterContainer[Character1] == StringTarget.CharacterContainer[Character2]) ++Character2;
                else Character1 -= Character2, Character2 = 0;
            if (Character2 == StringTarget.CharacterSize) return Character1 - StringTarget.CharacterSize;
            return -1;
        }

        String doReplace(const String &StringTarget, const String &StringSource) noexcept {
            std::basic_stringstream<char16_t> CharacterStream;
            std::u16string StringSource16(StringSource.toU16String());
            for (intmax_t Character1 = 0, Character2;Character1 < CharacterSize;++Character1) {
                for (Character2 = 0; Character2 < StringTarget.CharacterSize && CharacterContainer[Character1 + Character2] == StringTarget.CharacterContainer[Character2]; ++Character2);
                if (Character2 == StringTarget.CharacterSize) {
                    CharacterStream << StringSource16;
                    Character1 += StringTarget.CharacterSize - 1;
                } else CharacterStream << CharacterContainer[Character1];
            }
            return CharacterStream.str();
        }

        String doStrip(char16_t CharacterSource) const noexcept {
            std::basic_stringstream<char16_t> CharacterStream;
            for (intmax_t CharacterIndex = 0; CharacterIndex < CharacterSize; ++CharacterIndex)
                if (CharacterContainer[CharacterIndex] != CharacterSource) CharacterStream << CharacterContainer[CharacterIndex];
            return CharacterStream.str();
        }

        String doStrip(const String &StringTarget) noexcept {
            std::basic_stringstream<char16_t> StringStream;
            for (intmax_t Character1 = 0, Character2;Character1 < CharacterSize;++Character1) {
                for (Character2 = 0; Character2 < StringTarget.CharacterSize && CharacterContainer[Character1 + Character2] == StringTarget.CharacterContainer[Character2]; ++Character2);
                if (Character2 == StringTarget.CharacterSize) Character1 += StringTarget.CharacterSize - 1;
                else StringStream << CharacterContainer[Character1];
            }
            return StringStream.str();
        }

        String doTruncate(intmax_t CharacterStart, intmax_t CharacterStop) const;

        char16_t getCharacter(intmax_t CharacterIndex) const;

        intmax_t getCharacterSize() const noexcept {
            return CharacterSize;
        }

        intmax_t hashCode() const noexcept override {
            uintmax_t HashCode = 0;
            for (intmax_t ElementIndex = 0; ElementIndex < CharacterSize; ++ElementIndex)
                HashCode = (HashCode << 5) - HashCode + CharacterContainer[ElementIndex];
            return (intmax_t) HashCode;
        }

        bool isContains(char16_t CharacterTarget) const noexcept {
            return doFind(CharacterTarget) != -1;
        }

        bool isContains(const String &StringTarget) const noexcept {
            return doFind(StringTarget) != -1;
        }

        bool isEmpty() const noexcept {
            return CharacterSize == 0;
        }

        bool isEndswith(const String &StringSuffix) const noexcept {
            if (StringSuffix.CharacterSize > CharacterSize) return false;
            for (intmax_t CharacterIndex = 0; CharacterIndex < StringSuffix.CharacterSize; ++CharacterIndex)
                if (CharacterContainer[CharacterIndex + CharacterSize - StringSuffix.CharacterSize] !=
                    StringSuffix.CharacterContainer[CharacterIndex])
                    return false;
            return true;
        }

        bool isNull() const noexcept {
            return CharacterContainer == nullptr;
        }

        bool isStartswith(const String &StringPrefix) const noexcept {
            if (StringPrefix.CharacterSize > CharacterSize) return false;
            for (intmax_t CharacterIndex = 0; CharacterIndex < StringPrefix.CharacterSize; ++CharacterIndex)
                if (CharacterContainer[CharacterIndex] != StringPrefix.CharacterContainer[CharacterIndex]) return false;
            return true;
        }

        String &operator=(const String &StringSource) noexcept {
            if (&StringSource == this) return *this;
            doAssign(StringSource);
            return *this;
        }

        String toLowerCase() const noexcept {
            std::basic_stringstream<char16_t> CharacterStream;
            for (intmax_t CharacterIndex = 0;CharacterIndex < CharacterSize;++CharacterIndex)
                CharacterStream << towlower(CharacterContainer[CharacterIndex]);
            return CharacterStream.str();
        }

        String toUpperCase() const noexcept {
            std::basic_stringstream<char16_t> CharacterStream;
            for (intmax_t CharacterIndex = 0;CharacterIndex < CharacterSize;++CharacterIndex)
                CharacterStream << towupper(CharacterContainer[CharacterIndex]);
            return CharacterStream.str();
        }

        String toString() const noexcept override {
            return *this;
        }

        std::string toU8String() const noexcept {
            static std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> StringConverter;
            return StringConverter.to_bytes(CharacterContainer, CharacterContainer + CharacterSize);
        }

        std::u16string toU16String() const noexcept {
            return CharacterContainer;
        }

        std::u32string toU32String() const noexcept {
            static std::wstring_convert<std::codecvt_utf16<char32_t>, char32_t> StringConverter;
            return StringConverter.from_bytes((const char*) CharacterContainer, (const char*) (CharacterContainer + CharacterSize));
        }

        std::wstring toWString() const noexcept {
            std::u16string U16String = CharacterContainer;
            return std::wstring(U16String.begin(), U16String.end());
        }

        template<typename T> requires std::is_integral<T>::value
        static String valueOf(T ObjectSource, unsigned short NumberRadix = 10) noexcept {
            static std::map<unsigned short, char16_t> NumberDigitMapping;
            if (NumberDigitMapping.empty()) {
                for (unsigned short NumberDigit = 0; NumberDigit < 10; ++NumberDigit)
                    NumberDigitMapping[NumberDigit] = char16_t(NumberDigit + 48);
                for (unsigned short NumberDigit = 0; NumberDigit < 26; ++NumberDigit)
                    NumberDigitMapping[NumberDigit + 10] = char16_t(NumberDigit + 65);
            }
            std::deque<char16_t> NumberDeque;
            bool NumberNegative = false;
            if (ObjectSource < 0) NumberNegative = true, ObjectSource = -ObjectSource;
            while (ObjectSource) {
                NumberDeque.push_front(NumberDigitMapping[ObjectSource % NumberRadix]);
                ObjectSource /= NumberRadix;
            }
            if (NumberNegative) NumberDeque.push_front(u'-');
            return std::u16string(NumberDeque.begin(), NumberDeque.end());
        }

        template<typename T> requires std::is_base_of<Object, T>::value
        static String valueOf(const T &ObjectSource) noexcept {
            return ObjectSource.toString();
        }
    };
}
