#pragma once

#include <Core/Object.hpp>
#include <cwctype>
#include <string>

namespace eLibrary::Core {
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
            return iswalpha(CharacterValue) != 0;
        }

        bool isDigit() const noexcept {
            return iswdigit(CharacterValue) != 0;
        }

        bool isLowerCase() const noexcept {
            return iswlower(CharacterValue) != 0;
        }

        bool isNull() const noexcept {
            return CharacterValue == char16_t();
        }

        bool isUpperCase() const noexcept {
            return iswupper(CharacterValue) != 0;
        }

        explicit operator char16_t() const noexcept {
            return CharacterValue;
        }

        Character toLowerCase() const noexcept {
            return {(char16_t) towlower(CharacterValue)};
        }

        eLibraryAPI uint8_t toNumber(uint8_t) const;

        eLibraryAPI String toString() const noexcept override;

        Character toUpperCase() const noexcept {
            return {(char16_t) towupper(CharacterValue)};
        }

        eLibraryAPI static Character valueOf(uint8_t, uint8_t);
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
        doEnableMoveAssignConstruct(String)

        constexpr String() noexcept = default;

        eLibraryAPI String(const Character&) noexcept;

        eLibraryAPI String(const std::string&) noexcept;

        eLibraryAPI String(const std::u16string&) noexcept;

        eLibraryAPI String(const std::u32string&) noexcept;

        eLibraryAPI String(const std::wstring&) noexcept;

        ~String() noexcept {
            if (CharacterContainer && CharacterSize) {
                CharacterSize = 0;
                delete[] CharacterContainer;
                CharacterContainer = nullptr;
            }
        }

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

        eLibraryAPI std::string toU8String() const noexcept;

        eLibraryAPI std::u16string toU16String() const noexcept;

        eLibraryAPI std::u32string toU32String() const noexcept;

        eLibraryAPI std::wstring toWString() const noexcept;

        template<Arithmetic T>
        static String valueOf(T ObjectSource) noexcept {
            return std::to_string(ObjectSource);
        }

        template<typename T>
        static String valueOf() noexcept {
            constexpr std::string_view ObjectNameSource = __PRETTY_FUNCTION__;
            return {std::string{ObjectNameSource.data() + 74, ObjectNameSource.data() + ObjectNameSource.size() - 1}};
        }

        template<ObjectDerived T>
        static String valueOf(const T &ObjectSource) noexcept {
            return ObjectSource.toString();
        }
    };

    class StringStream final : public Object {
    private:
        uintmax_t CharacterCapacity = 0;
        uintmax_t CharacterSize = 0;
        Character *CharacterContainer = nullptr;

        doDisableCopyAssignConstruct(StringStream)
    public:
        constexpr StringStream() noexcept = default;

        eLibraryAPI explicit StringStream(uintmax_t CharacterCapacitySource);

        ~StringStream() noexcept {
            doClear();
        }

        eLibraryAPI void addCharacter(const Character&);

        eLibraryAPI void addString(const String &StringSource);

        void doClear() noexcept {
            CharacterCapacity = 0;
            CharacterSize = 0;
            delete[] CharacterContainer;
            CharacterContainer = nullptr;
        }

        eLibraryAPI String toString() const noexcept override;
    };
}

#if __has_include(<format>) && !eLibraryCompiler(AppleClang)
#include <format>

template<eLibrary::Core::ObjectDerived ObjectT, typename CharacterT>
struct std::formatter<ObjectT, CharacterT> : public std::formatter<std::string, CharacterT> {
public:
    template<typename ContextT>
    auto format(const ObjectT &ObjectSource, ContextT &ObjectContext) const noexcept {
        return std::formatter<std::string, CharacterT>::format(ObjectSource.toString().toU8String(), ObjectContext);
    }
};
#endif
