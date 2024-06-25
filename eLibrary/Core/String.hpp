#pragma once

#ifndef eLibraryHeaderCoreString
#define eLibraryHeaderCoreString

#include <Core/Memory.hpp>
#include <Core/StringUnicode.hpp>
#include <string>

namespace eLibrary::Core {
    class CharacterLatin1;
    class CharacterUcs4;
    template<typename>
    class Reference;
    class StringBuilder;

    class CharacterUtility final : public NonConstructable {
    public:
        static bool isAlpha(char32_t CharacterValue) noexcept {
            return CharacterDerivedCore::doQuery(CharacterValue) & CharacterDerivedCore::DerivedProperty_Alphabetic;
        }

        static bool isDigit(char32_t CharacterValue) noexcept {
            switch (CharacterGeneralCategory::doQuery(CharacterValue)) {
                case CharacterGeneralCategory::Nd:
                case CharacterGeneralCategory::Nl:
                case CharacterGeneralCategory::No:
                    return true;
                default:
                    return false;
            }
        }

        static bool isHighSurrogate(char32_t CharacterValue) noexcept {
            return (CharacterValue & 0xFFFFFC00) == 0xD800;
        }

        static bool isLetter(char32_t CharacterValue) noexcept {
            switch (CharacterGeneralCategory::doQuery(CharacterValue)) {
                case CharacterGeneralCategory::Ll:
                case CharacterGeneralCategory::Lm:
                case CharacterGeneralCategory::Lo:
                case CharacterGeneralCategory::Lu:
                case CharacterGeneralCategory::Lt:
                    return true;
                default:
                    return false;
            }
        }

        static bool isLowerCase(char32_t CharacterValue) noexcept {
            return CharacterDerivedCore::doQuery(CharacterValue) & CharacterDerivedCore::DerivedProperty_Lowercase;
        }

        static bool isLowSurrogate(char32_t CharacterValue) noexcept {
            return (CharacterValue & 0xFFFFFC00) == 0xDC00;
        }

        static bool isNonCharacter(char32_t CharacterValue) noexcept {
            return CharacterValue >= 0xFDD0 && CharacterValue <= 0xFDEF;
        }

        static bool isNull(char32_t CharacterValue) noexcept {
            return CharacterValue == char32_t();
        }

        static bool isSpace(char32_t CharacterValue) noexcept {
            switch (CharacterGeneralCategory::doQuery(CharacterValue)) {
                case CharacterGeneralCategory::Zl:
                case CharacterGeneralCategory::Zp:
                case CharacterGeneralCategory::Zs:
                    return true;
                default:
                    return CharacterValue == 0x20 || (CharacterValue <= 0x0D && CharacterValue >= 0x09) || CharacterValue == 0x85 || CharacterValue == 0xA0;
            }
        }

        static bool isSurrogate(char32_t CharacterValue) noexcept {
            return CharacterValue >= 0xD800 && CharacterValue <= 0xDFFF;
        }

        static bool isUpperCase(char32_t CharacterValue) noexcept {
            return CharacterDerivedCore::doQuery(CharacterValue) & CharacterDerivedCore::DerivedProperty_Uppercase;
        }

        static char32_t toLowerCase(char32_t CharacterValue) noexcept {
            return CharacterCaseMappingSimple::isContains(CharacterValue) ? CharacterCaseMappingSimple::doQuery(CharacterValue)[1] : CharacterValue;
        }

        static char32_t toUpperCase(char32_t CharacterValue) noexcept {
            return CharacterCaseMappingSimple::isContains(CharacterValue) ? CharacterCaseMappingSimple::doQuery(CharacterValue)[0] : CharacterValue;
        }
    };

    /**
     * Support for character storage and operation
     */
    class Character final : public Object {
    private:
        char16_t CharacterValue;
    public:
        constexpr Character(char16_t CharacterSource=char16_t()) noexcept : CharacterValue(CharacterSource) {}

        intmax_t doCompare(const Character &CharacterSource) const noexcept {
            return (intmax_t) CharacterValue - CharacterSource.CharacterValue;
        }

        uintmax_t hashCode() const noexcept override {
            return CharacterValue;
        }

        bool isAlpha() const noexcept {
            return CharacterDerivedCore::doQuery(CharacterValue) & CharacterDerivedCore::DerivedProperty_Alphabetic;
        }

        bool isDigit() const noexcept {
            return CharacterUtility::isDigit(CharacterValue);
        }

        bool isHighSurrogate() const noexcept {
            return CharacterUtility::isHighSurrogate(CharacterValue);
        }

        bool isLetter() const noexcept {
            return CharacterUtility::isLetter(CharacterValue);
        }

        bool isLowerCase() const noexcept {
            return CharacterUtility::isLowerCase(CharacterValue);
        }

        bool isLowSurrogate() const noexcept {
            return CharacterUtility::isLowSurrogate(CharacterValue);
        }

        bool isNonCharacter() const noexcept {
            return CharacterUtility::isNonCharacter(CharacterValue);
        }

        bool isNull() const noexcept {
            return CharacterUtility::isNull(CharacterValue);
        }

        bool isSpace() const noexcept {
            return CharacterUtility::isSpace(CharacterValue);
        }

        bool isSurrogate() const noexcept {
            return CharacterUtility::isSurrogate(CharacterValue);
        }

        bool isUpperCase() const noexcept {
            return CharacterUtility::isUpperCase(CharacterValue);
        }

        explicit operator char16_t() const noexcept {
            return CharacterValue;
        }

        eLibraryAPI CharacterLatin1 toLantin1() const noexcept;

        Character toLowerCase() const noexcept {
            return (char16_t) CharacterUtility::toLowerCase(CharacterValue);
        }

        eLibraryAPI uint8_t toNumber(uint8_t) const;

        eLibraryAPI String toString() const noexcept override;

        eLibraryAPI CharacterUcs4 toUcs4() const noexcept;

        Character toUpperCase() const noexcept {
            return (char16_t) CharacterUtility::toUpperCase(CharacterValue);
        }

        eLibraryAPI static Character valueOf(uint8_t, uint8_t);
    };

    class CharacterLatin1 final : public Object {
    private:
        char CharacterValue;
    public:
        constexpr CharacterLatin1(char CharacterSource = char()) noexcept : CharacterValue(CharacterSource) {}

        intmax_t doCompare(const CharacterLatin1 &CharacterSource) const noexcept {
            return (intmax_t) CharacterValue - CharacterSource.CharacterValue;
        }

        uintmax_t hashCode() const noexcept override {
            return CharacterValue;
        }

        explicit operator char() const noexcept {
            return CharacterValue;
        }

        Character toCharacter() const noexcept {
            return {(char16_t) CharacterValue};
        }

        eLibraryAPI String toString() const noexcept override;
    };

    class CharacterUcs4 final : public Object {
    private:
        char32_t CharacterValue;
    public:
        constexpr CharacterUcs4(char32_t CharacterSource = char32_t()) noexcept : CharacterValue(CharacterSource) {}

        intmax_t doCompare(const CharacterUcs4 &CharacterSource) const noexcept {
            return (intmax_t) CharacterValue - CharacterSource.CharacterValue;
        }

        uintmax_t hashCode() const noexcept override {
            return CharacterValue;
        }

        explicit operator char32_t() const noexcept {
            return CharacterValue;
        }

        eLibraryAPI String toString() const noexcept override;
    };

    /**
     * Support for string storage and operation
     */
    class String final : public Object {
    private:
        intmax_t CharacterSize = 0;
        Character *CharacterContainer = nullptr;
        mutable MemoryAllocator<Character> CharacterAllocator;

        eLibraryAPI String(const StringBuilder&) noexcept;

        friend class StringBuilder;
    public:
        doEnableCopyAssignConstruct(String)
        doEnableMoveAssignConstruct(String)

        constexpr String() noexcept = default;

        eLibraryAPI String(const ::std::u16string&) noexcept;

        eLibraryAPI ~String() noexcept;

        eLibraryAPI void doAssign(const String&) noexcept;

        eLibraryAPI void doAssign(String&&) noexcept;

        intmax_t doCompare(const String &StringOther) const noexcept {
            if (CharacterSize != StringOther.CharacterSize) return CharacterSize - StringOther.CharacterSize;
            for (intmax_t CharacterIndex = 0; CharacterIndex < CharacterSize; ++CharacterIndex)
                if (CharacterContainer[CharacterIndex].doCompare(StringOther.CharacterContainer[CharacterIndex]))
                    return CharacterContainer[CharacterIndex].doCompare(StringOther.CharacterContainer[CharacterIndex]);
            return 0;
        }

        eLibraryAPI String doConcat(const Character &CharacterSource) const noexcept;

        eLibraryAPI String doConcat(const String &StringOther) const noexcept;

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

        eLibraryAPI [[deprecated("Not implemented")]] Reference<String> doIntern() const;

        eLibraryAPI String doRepeat(uintmax_t StringCount) const noexcept;

        eLibraryAPI String doReplace(const String &StringTarget, const String &StringSource) const noexcept;

        eLibraryAPI String doReverse() const noexcept;

        eLibraryAPI String doStrip(const Character &CharacterSource) const noexcept;

        eLibraryAPI String doStrip(const String &StringTarget) noexcept;

        eLibraryAPI String doTruncate(intmax_t CharacterStart, intmax_t CharacterStop) const;

        eLibraryAPI Character getCharacter(intmax_t CharacterIndex) const;

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

        eLibraryAPI String toLowerCase() const noexcept;

        eLibraryAPI String toUpperCase() const noexcept;

        String toString() const noexcept override {
            return *this;
        }

        eLibraryAPI ::std::string toU8String() const noexcept;

        eLibraryAPI ::std::u16string toU16String() const noexcept;

        eLibraryAPI ::std::u32string toU32String() const noexcept;

        eLibraryAPI ::std::wstring toWString() const noexcept;

        static eLibraryAPI String valueOf(const Character&) noexcept;

        static String valueOf(const CharacterLatin1 &ObjectSource) noexcept {
            return ObjectSource.toString();
        }

        static String valueOf(const CharacterUcs4 &ObjectSource) noexcept {
            return ObjectSource.toString();
        }

        static eLibraryAPI String valueOf(const ::std::string&);

        static String valueOf(const ::std::u16string &StringSource) noexcept {
            return {StringSource};
        }

        static eLibraryAPI String valueOf(const ::std::u32string&) noexcept;

        static eLibraryAPI String valueOf(const ::std::wstring&) noexcept;

        template<Type::Arithmetic T>
        static String valueOf(T ObjectSource) noexcept {
            return valueOf(::std::to_string(ObjectSource));
        }

        template<ObjectDerived T>
        static String valueOf(const T &ObjectSource) noexcept {
            return ObjectSource.toString();
        }

        template<ObjectDerived T>
        static String valueOf(const T *ObjectSource) noexcept {
            return ObjectSource->toString();
        }
    };

    class StringBuilder final : public Object, public NonCopyable {
    private:
        uintmax_t CharacterCapacity = 0;
        uintmax_t CharacterSize = 0;
        Character *CharacterContainer = nullptr;
        mutable MemoryAllocator<Character> CharacterAllocator;

        friend class String;
    public:
        StringBuilder() noexcept = default;

        explicit StringBuilder(uintmax_t CharacterCapacitySource) noexcept {
            doReserve(CharacterCapacitySource);
        }

        ~StringBuilder() noexcept {
            doClear();
        }

        eLibraryAPI void addCharacter(const Character&) noexcept;

        eLibraryAPI void addString(const String&) noexcept;

        eLibraryAPI void doClear() noexcept;

        eLibraryAPI void doReserve(uintmax_t) noexcept;

        eLibraryAPI String toString() const noexcept override {
            return {*this};
        }
    };

    class [[deprecated("Not implemented")]] StringPool final : public Object, public NonCopyable, public NonMovable {
    private:
        struct PoolPage {
            uint8_t *PageData = nullptr;
            uintmax_t PageSize = 0;
        } *PoolPageList = nullptr;
    public:
        eLibraryAPI Reference<String> doIntern(const String&);

        static StringPool &getInstance() noexcept {
            static StringPool PoolInstance;
            return PoolInstance;
        }
    };

    inline namespace Literal {
        eLibraryAPI Character operator"" _C(char16_t CharacterSource) noexcept;
        eLibraryAPI CharacterLatin1 operator"" _CL1(char CharacterSource) noexcept;
        eLibraryAPI String operator"" _S(const char16_t *StringSource, size_t StringSize);
    }
}

#if __has_include(<format>) && !eLibraryCompiler(AppleClang)
#include <format>

template<eLibrary::Core::ObjectDerived To, typename Tc>
struct std::formatter<To, Tc> : public ::std::formatter<std::string, Tc> {
public:
    template<typename ContextT>
    auto format(const To &ObjectSource, ContextT &ObjectContext) const noexcept {
        return ::std::formatter<::std::string, Tc>::format(ObjectSource.toString().toU8String(), ObjectContext);
    }
};
#endif

#endif
