#pragma once

#include <algorithm>
#include <codecvt>
#include <cstring>
#include <string>

#include <Core/Object.hpp>

namespace eLibrary::Core {
    /**
     * Support for character storage and operation
     */
    class Character final : public Object {
    private:
        char16_t CharacterValue;
    public:
        constexpr Character() noexcept : CharacterValue(char16_t()) {}

        constexpr Character(char16_t CharacterSource) noexcept : CharacterValue(CharacterSource) {}

        intmax_t doCompare(const Character &CharacterSource) const noexcept {
            return CharacterValue - CharacterSource.CharacterValue;
        }

        bool isAlpha() const noexcept {
            return iswalpha(CharacterValue);
        }

        bool isLowerCase() const noexcept {
            return iswlower(CharacterValue);
        }

        bool isNull() const noexcept {
            return CharacterValue == char16_t();
        }

        bool isNumber() const noexcept {
            return CharacterValue >= u'0' && CharacterValue <= u'9';
        }

        bool isUpperCase() const noexcept {
            return iswupper(CharacterValue);
        }

        explicit operator char16_t() const noexcept {
            return CharacterValue;
        }

        Character toLowerCase() const noexcept {
            return {towlower(CharacterValue)};
        }

        Character toUpperCase() const noexcept {
            return {towupper(CharacterValue)};
        }
    };

    /**
     * Support for string storage and operation
     */
    class String final : public Object {
    private:
        intmax_t CharacterSize = 0;
        Character *CharacterContainer = nullptr;

        friend class StringStream;
    public:
        doEnableCopyAssignConstruct(String)

        constexpr String() noexcept = default;

        explicit String(const Character&);

        String(const std::string&);

        String(const std::u16string&);

        String(const std::u32string&);

        String(const std::wstring&);

        ~String() noexcept {
            if (CharacterContainer && CharacterSize) {
                CharacterSize = 0;
                delete[] CharacterContainer;
                CharacterContainer = nullptr;
            }
        }

        void doAssign(const String&);

        intmax_t doCompare(const String &StringOther) const noexcept {
            if (CharacterSize != StringOther.CharacterSize) return CharacterSize - StringOther.CharacterSize;
            for (intmax_t CharacterIndex = 0; CharacterIndex < CharacterSize; ++CharacterIndex)
                if (CharacterContainer[CharacterIndex].doCompare(StringOther.CharacterContainer[CharacterIndex]))
                    return CharacterContainer[CharacterIndex].doCompare(StringOther.CharacterContainer[CharacterIndex]);
            return 0;
        }

        String doConcat(const Character &CharacterSource) const noexcept;

        String doConcat(const String &StringOther) const noexcept;

        intmax_t doFind(const Character &CharacterSource) const noexcept {
            for (intmax_t CharacterIndex = 0;CharacterIndex < CharacterSize;++CharacterIndex)
                if (!Objects::doCompare(CharacterContainer[CharacterIndex], CharacterSource)) return CharacterIndex;
            return -1;
        }

        intmax_t doFind(const String &StringTarget) const noexcept {
            intmax_t Character1, Character2;
            for (Character1 = 0, Character2 = 0;Character1 < CharacterSize && Character2 < StringTarget.CharacterSize; ++Character1)
                if (!Objects::doCompare(CharacterContainer[Character1], StringTarget.CharacterContainer[Character2])) ++Character2;
                else Character1 -= Character2, Character2 = 0;
            if (Character2 == StringTarget.CharacterSize) return Character1 - StringTarget.CharacterSize;
            return -1;
        }

        String doReplace(const String &StringTarget, const String &StringSource) const noexcept;

        String doReverse() const noexcept;

        String doStrip(const Character &CharacterSource) const noexcept;

        String doStrip(const String &StringTarget) noexcept;

        String doTruncate(intmax_t CharacterStart, intmax_t CharacterStop) const;

        Character getCharacter(intmax_t CharacterIndex) const;

        intmax_t getCharacterSize() const noexcept {
            return CharacterSize;
        }

        uintmax_t hashCode() const noexcept override {
            uintmax_t CharacterCode = 0;
            for (intmax_t ElementIndex = 0; ElementIndex < CharacterSize; ++ElementIndex)
                CharacterCode = CharacterCode * 29 + CharacterContainer[ElementIndex].hashCode();
            return CharacterCode;
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
                if (Objects::doCompare(CharacterContainer[CharacterIndex + CharacterSize - StringSuffix.CharacterSize], StringSuffix.CharacterContainer[CharacterIndex]))
                    return false;
            return true;
        }

        bool isNull() const noexcept {
            return CharacterContainer == nullptr;
        }

        bool isStartswith(const String &StringPrefix) const noexcept {
            if (StringPrefix.CharacterSize > CharacterSize) return false;
            for (intmax_t CharacterIndex = 0; CharacterIndex < StringPrefix.CharacterSize; ++CharacterIndex)
                if (Objects::doCompare(CharacterContainer[CharacterIndex], StringPrefix.CharacterContainer[CharacterIndex])) return false;
            return true;
        }

        String toLowerCase() const noexcept;

        String toUpperCase() const noexcept;

        String toString() const noexcept override {
            return *this;
        }

        std::string toU8String() const noexcept {
            static std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> StringConverter;
            char16_t CharacterBuffer[CharacterSize + 1];
            for (intmax_t CharacterIndex = 0;CharacterIndex < CharacterSize;++CharacterIndex)
                CharacterBuffer[CharacterIndex] = (char16_t) CharacterContainer[CharacterIndex];
            CharacterBuffer[CharacterSize] = char();
            return StringConverter.to_bytes(CharacterBuffer, CharacterBuffer + CharacterSize);
        }

        std::u16string toU16String() const noexcept {
            char16_t CharacterBuffer[CharacterSize + 1];
            for (intmax_t CharacterIndex = 0;CharacterIndex < CharacterSize;++CharacterIndex)
                CharacterBuffer[CharacterIndex] = (char16_t) CharacterContainer[CharacterIndex];
            CharacterBuffer[CharacterSize] = char16_t();
            return {CharacterBuffer, (size_t) CharacterSize};
        }

        std::u32string toU32String() const noexcept {
            static std::wstring_convert<std::codecvt_utf16<char32_t>, char32_t> StringConverter;
            std::u16string StringSource(toU16String());
            return StringConverter.from_bytes((const char*) StringSource.begin().base(), (const char*) StringSource.end().base());
        }

        std::wstring toWString() const noexcept {
            wchar_t CharacterBuffer[CharacterSize + 1];
            for (intmax_t CharacterIndex = 0;CharacterIndex < CharacterSize;++CharacterIndex)
                CharacterBuffer[CharacterIndex] = (wchar_t) (char16_t) CharacterContainer[CharacterIndex];
            CharacterBuffer[CharacterSize] = wchar_t();
            return {CharacterBuffer, (size_t) CharacterSize};
        }

        template<Arithmetic T>
        static String valueOf(T ObjectSource) noexcept {
            return std::to_string(ObjectSource);
        }

        template<ObjectDerived T>
        static String valueOf(const T &ObjectSource) noexcept {
            return ObjectSource.toString();
        }
    };

    class StringStream final : public Object {
    private:
        intmax_t CharacterCapacity = 0;
        intmax_t CharacterSize = 0;
        Character *CharacterContainer = nullptr;

        doDisableCopyAssignConstruct(StringStream)
    public:
        constexpr StringStream() noexcept = default;

        ~StringStream() noexcept {
            doClear();
        }

        void addCharacter(const Character&);

        void addString(const String &StringSource) noexcept {
            for (intmax_t CharacterIndex = 0;CharacterIndex < StringSource.CharacterSize;++CharacterIndex)
                addCharacter(StringSource.CharacterContainer[CharacterIndex]);
        }

        void doClear() noexcept {
            CharacterCapacity = 0;
            CharacterSize = 0;
            delete[] CharacterContainer;
            CharacterContainer = nullptr;
        }

        String toString() const noexcept override {
            char16_t CharacterBuffer[CharacterSize + 1];
            for (intmax_t CharacterIndex = 0;CharacterIndex < CharacterSize;++CharacterIndex)
                CharacterBuffer[CharacterIndex] = (char16_t) CharacterContainer[CharacterIndex];
            CharacterBuffer[CharacterSize] = char16_t();
            return {{CharacterBuffer, (size_t) CharacterSize}};
        }
    };
}
