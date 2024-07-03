#include <Core/Container.hpp>
#include <Core/Mathematics.hpp>

namespace eLibrary::Core {
    CharacterLatin1 Character::toLantin1() const noexcept {
        return {CharacterValue > 0xFF ? '\0' : (char) CharacterValue};
    }

    uint8_t Character::toNumber(uint8_t NumberRadix) const {
        if (NumberRadix < 2 || NumberRadix > 36) doThrowChecked(ArithmeticException, u"Character::toNumber(uint8_t) NumberRadix"_S);
        if (isDigit()) {
            if (CharacterValue - 48 >= NumberRadix)
                doThrowChecked(Exception, u"Character::toNumber(uint8_t) NumberRadix"_S);
            return CharacterValue - 48;
        } else if (!isAlpha()) doThrowChecked(Exception, u"Character::toNumber(uint8_t) isAlpha"_S);
        if (toUpperCase().CharacterValue - 55 >= NumberRadix)
            doThrowChecked(Exception, u"Character::toNumber(uint8_t) NumberRadix"_S);
        return toUpperCase().CharacterValue - 55;
    }

    String Character::toString() const noexcept {
        StringBuilder StringResult;
        StringResult.addCharacter(CharacterValue);
        return StringResult.toString();
    }

    CharacterUcs4 Character::toUcs4() const noexcept {
        return {CharacterValue};
    }

    Character Character::valueOf(uint8_t NumberSource, uint8_t NumberRadix) {
        if (NumberRadix < 2 || NumberRadix > 36) doThrowChecked(ArithmeticException, u"Character::valueOf(uint8_t, uint8_t) NumberRadix"_S);
        if (NumberSource >= NumberRadix) doThrowChecked(Exception, u"Character::valueOf(uint8_t, uint8_t) NumberSource"_S);
        if (NumberSource <= 10) return {char16_t(NumberSource + 48)};
        return {char16_t(NumberSource + 55)};
    }

    String CharacterLatin1::toString() const noexcept {
        return toCharacter().toString();
    }

    String CharacterUcs4::toString() const noexcept {
        StringBuilder StringResult;
        if (CharacterValue <= 0xFFFF) StringResult.addCharacter(CharacterValue);
        else {
            char32_t CharacterProxy = CharacterValue - 0x10000;
            StringResult.addCharacter(0xD800 + static_cast<char16_t>((CharacterProxy >> 10) & 0x3FF));
            StringResult.addCharacter(0xDC00 + static_cast<char16_t>(CharacterProxy & 0x3FF));
        }
        return StringResult.toString();
    }

    String::String(const eLibrary::Core::StringBuilder &StringSource) noexcept : CharacterSize((intmax_t) StringSource.CharacterSize) {
        CharacterContainer = CharacterAllocator.doAllocate(CharacterSize + 1);
        Collections::doCopy(StringSource.CharacterContainer, CharacterSize, CharacterContainer);
        CharacterContainer[CharacterSize] = Character();
    }

    String::String(const ::std::u16string &StringSource) noexcept : CharacterSize((intmax_t) StringSource.size()) {
        CharacterContainer = CharacterAllocator.doAllocate(CharacterSize + 1);
        Collections::doCopy(StringSource.begin(), StringSource.end(), CharacterContainer);
        CharacterContainer[CharacterSize] = Character();
    }

    String::~String() noexcept {
        CharacterAllocator.doDeallocate(CharacterContainer, CharacterSize + 1);
        CharacterSize = 0;
        CharacterContainer = nullptr;
    }

    void String::doAssign(const String &StringSource) noexcept {
        if (Objects::getAddress(StringSource) == this) return;
        CharacterAllocator.doDeallocate(CharacterContainer, CharacterSize + 1);
        CharacterContainer = CharacterAllocator.doAllocate((CharacterSize = StringSource.CharacterSize) + 1);
        Collections::doCopy(StringSource.CharacterContainer, CharacterSize, CharacterContainer);
        CharacterContainer[CharacterSize] = Character();
    }

    void String::doAssign(String &&StringSource) noexcept {
        if (Objects::getAddress(StringSource) == this) return;
        CharacterAllocator.doDeallocate(CharacterContainer, CharacterSize + 1);
        CharacterContainer = StringSource.CharacterContainer;
        CharacterSize = StringSource.CharacterSize;
        StringSource.CharacterContainer = nullptr;
        StringSource.CharacterSize = 0;
    }

    String String::doConcat(const Character &CharacterSource) const noexcept {
        StringBuilder CharacterStream(CharacterSize + 1);
        CharacterStream.addString(*this);
        CharacterStream.addCharacter(CharacterSource);
        return CharacterStream.toString();
    }

    String String::doConcat(const String &StringOther) const noexcept {
        StringBuilder CharacterStream(CharacterSize + StringOther.CharacterSize);
        CharacterStream.addString(*this);
        CharacterStream.addString(StringOther);
        return CharacterStream.toString();
    }

    Reference<String> String::doIntern() const {
        return {StringPool::getInstance().doIntern(*this)};
    }

    String String::doRepeat(uintmax_t StringCount) const noexcept {
        StringBuilder CharacterStream;
        CharacterStream.doReserve(getCharacterSize() * StringCount);
        for (uintmax_t StringIndex = 0;StringIndex < StringCount;++StringIndex)
            CharacterStream.addString(*this);
        return CharacterStream.toString();
    }

    String String::doReplace(const String &StringTarget, const String &StringSource) const noexcept {
        StringBuilder CharacterStream;
        for (intmax_t Character1 = 0, Character2;Character1 < CharacterSize;++Character1) {
            for (Character2 = 0; Character2 < StringTarget.CharacterSize && !CharacterContainer[Character1 + Character2].doCompare(StringTarget.CharacterContainer[Character2]); ++Character2);
            if (Character2 == StringTarget.CharacterSize) {
                CharacterStream.addString(StringSource);
                Character1 += StringTarget.CharacterSize - 1;
            } else CharacterStream.addCharacter(CharacterContainer[Character1]);
        }
        return CharacterStream.toString();
    }

    String String::doReverse() const noexcept {
        StringBuilder CharacterStream(CharacterSize);
        for (intmax_t CharacterIndex = 0;CharacterIndex < CharacterSize;++CharacterIndex)
            CharacterStream.addCharacter(CharacterContainer[CharacterSize - CharacterIndex - 1]);
        return CharacterStream.toString();
    }

    String String::doStrip(const Character &CharacterSource) const noexcept {
        StringBuilder CharacterStream;
        for (intmax_t CharacterIndex = 0; CharacterIndex < CharacterSize; ++CharacterIndex)
            if (Objects::doCompare(CharacterContainer[CharacterIndex], CharacterSource)) CharacterStream.addCharacter(CharacterContainer[CharacterIndex]);
        return CharacterStream.toString();
    }

    String String::doStrip(const String &StringTarget) noexcept {
        StringBuilder CharacterStream;
        for (intmax_t Character1 = 0, Character2;Character1 < CharacterSize;++Character1) {
            for (Character2 = 0; Character2 < StringTarget.CharacterSize && !Objects::doCompare(CharacterContainer[Character1 + Character2], StringTarget.CharacterContainer[Character2]); ++Character2);
            if (Character2 == StringTarget.CharacterSize) Character1 += StringTarget.CharacterSize - 1;
            else CharacterStream.addCharacter(CharacterContainer[Character1]);
        }
        return CharacterStream.toString();
    }

    String String::doTruncate(intmax_t CharacterStart, intmax_t CharacterStop) const {
        if (CharacterStart < 0) CharacterStart += CharacterSize;
        Collections::doCheckGE(CharacterStart, 0);
        Collections::doCheckL(CharacterStart, CharacterSize);
        if (CharacterStop < 0) CharacterStop += CharacterSize + 1;
        Collections::doCheckGE(CharacterStop, 0);
        Collections::doCheckLE(CharacterStop, CharacterSize);
        Collections::doCheckLE(CharacterStart, CharacterStop);
        StringBuilder CharacterStream(CharacterStop - CharacterStart);
        for (intmax_t CharacterIndex = CharacterStart;CharacterIndex < CharacterStop;++CharacterIndex)
            CharacterStream.addCharacter(CharacterContainer[CharacterIndex]);
        return CharacterStream.toString();
    }

    Character String::getCharacter(intmax_t CharacterIndex) const {
        if (CharacterIndex < 0) CharacterIndex += CharacterSize;
        Collections::doCheckGE(CharacterIndex, 0);
        Collections::doCheckL(CharacterIndex, CharacterSize);
        return CharacterContainer[CharacterIndex];
    }

    String String::toLowerCase() const noexcept {
        StringBuilder CharacterStream(CharacterSize);
        for (intmax_t CharacterIndex = 0;CharacterIndex < CharacterSize;++CharacterIndex)
            CharacterStream.addCharacter(CharacterContainer[CharacterIndex].toLowerCase());
        return CharacterStream.toString();
    }

    String String::toUpperCase() const noexcept {
        StringBuilder CharacterStream(CharacterSize);
        for (intmax_t CharacterIndex = 0;CharacterIndex < CharacterSize;++CharacterIndex)
            CharacterStream.addCharacter(CharacterContainer[CharacterIndex].toUpperCase());
        return CharacterStream.toString();
    }

    ::std::string String::toU8String() const noexcept {
        ArrayList<char> CharacterBuffer;
        for (intmax_t CharacterIndex = 0;CharacterIndex < CharacterSize;++CharacterIndex) {
            char32_t CharacterSource = (char16_t) CharacterContainer[CharacterIndex];
            if (CharacterSource <= 0x7F)
                CharacterBuffer.addElement((char) CharacterSource);
            else if (CharacterSource <= 0x7FF) {
                CharacterBuffer.addElement(char(0xC0 | ((CharacterSource >> 6) & 0x1F)));
                CharacterBuffer.addElement(char(0x80 | (CharacterSource & 0x3F)));
            } else {
                CharacterBuffer.addElement(char(0xE0 | ((CharacterSource >> 12) & 0x0F)));
                CharacterBuffer.addElement(char(0x80 | ((CharacterSource >> 6) & 0x3F)));
                CharacterBuffer.addElement(char(0x80 | (CharacterSource & 0x3F)));
            }
        }
        ::std::string CharacterResult(CharacterBuffer.begin(), CharacterBuffer.end());
        return CharacterResult;
    }

    ::std::u16string String::toU16String() const noexcept {
        Array<char16_t> CharacterBuffer(CharacterSize);
        for (intmax_t CharacterIndex = 0;CharacterIndex < CharacterSize;++CharacterIndex)
            CharacterBuffer.getElement(CharacterIndex) = (char16_t) CharacterContainer[CharacterIndex];
        ::std::u16string CharacterResult(CharacterBuffer.begin(), CharacterBuffer.end());
        return CharacterResult;
    }

    ::std::u32string String::toU32String() const noexcept {
        Array<char32_t> CharacterBuffer(CharacterSize);
        for (intmax_t CharacterIndex = 0;CharacterIndex < CharacterSize;++CharacterIndex)
            CharacterBuffer.getElement(CharacterIndex) = (char16_t) CharacterContainer[CharacterIndex];
        ::std::u32string CharacterResult(CharacterBuffer.begin(), CharacterBuffer.end());
        return CharacterResult;
    }

    ::std::wstring String::toWString() const noexcept {
        if constexpr (sizeof(std::wstring::value_type) == sizeof(char16_t)) {
            ::std::u16string StringSource(toU16String());
            return {StringSource.begin(), StringSource.end()};
        } else if constexpr (sizeof(std::wstring::value_type) == sizeof(char32_t)) {
            ::std::u32string StringSource(toU32String());
            return {StringSource.begin(), StringSource.end()};
        }
    }

    String String::fromStd(const ::std::string &StringSource) {
        StringBuilder StringResult;
        for (intmax_t CharacterIndex = 0;CharacterIndex < StringSource.size();) {
            char32_t CharacterValue;
            if ((StringSource[CharacterIndex] & 0b10000000) == 0) {
                CharacterValue = StringSource[CharacterIndex++];
            } else if ((StringSource[CharacterIndex] & 0b11100000) == 0b11000000) {
                CharacterValue = (StringSource[CharacterIndex++] & 0b00011111) << 6;
                CharacterValue |= (StringSource[CharacterIndex++] & 0b00111111);
            } else if ((StringSource[CharacterIndex] & 0b11110000) == 0b11100000) {
                CharacterValue = (StringSource[CharacterIndex++] & 0b00001111) << 12;
                CharacterValue |= (StringSource[CharacterIndex++] & 0b00111111) << 6;
                CharacterValue |= (StringSource[CharacterIndex++] & 0b00111111);
            } else doThrowChecked(Exception, u"String::valueOf(const ::std::string&) CharacterValue"_S);
            if (CharacterValue <= 0xFFFF) StringResult.addCharacter(CharacterValue);
            else {
                char32_t CharacterProxy = CharacterValue - 0x10000;
                StringResult.addCharacter(0xD800 + static_cast<char16_t>((CharacterProxy >> 10) & 0x3FF));
                StringResult.addCharacter(0xDC00 + static_cast<char16_t>(CharacterProxy & 0x3FF));
            }
        }
        return StringResult.toString();
    }

    String String::fromStd(const ::std::wstring &StringSource) noexcept {
        if constexpr (sizeof(std::wstring::value_type) == sizeof(char16_t)) {
            return fromStd((::std::u16string&) StringSource);
        } else if constexpr (sizeof(std::wstring::value_type) == sizeof(char32_t)) {
            return fromStd((::std::u32string&) StringSource);
        }
    }

    String String::fromStd(const std::u32string &StringSource) noexcept {
        StringBuilder StringResult;
        for (char32_t CharacterIndex : StringSource) {
            if (CharacterIndex <= 0xFFFF) StringResult.addCharacter(CharacterIndex);
            else {
                char32_t CharacterProxy = CharacterIndex - 0x10000;
                StringResult.addCharacter(0xD800 + static_cast<char16_t>((CharacterProxy >> 10) & 0x3FF));
                StringResult.addCharacter(0xDC00 + static_cast<char16_t>(CharacterProxy & 0x3FF));
            }
        }
        return StringResult.toString();
    }

    void StringBuilder::addCharacter(const Character &CharacterSource) noexcept {
        if (CharacterSize == CharacterCapacity)
            doReserve(CharacterCapacity ? CharacterCapacity << 1 : 1);
        CharacterContainer[CharacterSize++] = CharacterSource;
    }

    void StringBuilder::addString(const String &StringSource) noexcept {
        if (CharacterSize + StringSource.getCharacterSize() > CharacterCapacity)
            doReserve(Mathematics::doCeil2(CharacterSize + StringSource.getCharacterSize()));
        Collections::doCopy(StringSource.CharacterContainer, StringSource.getCharacterSize(), CharacterContainer + CharacterSize);
        CharacterSize += StringSource.getCharacterSize();
    }

    void StringBuilder::doClear() noexcept {
        CharacterCapacity = 0;
        CharacterSize = 0;
        CharacterAllocator.doDeallocate(CharacterContainer, CharacterSize + 1);
        CharacterContainer = nullptr;
    }

    void StringBuilder::doReserve(uintmax_t CharacterCapacityNew) noexcept {
        if (!CharacterCapacity) [[unlikely]] CharacterContainer = CharacterAllocator.doAllocate(CharacterCapacity = 1);
        if (CharacterCapacityNew > CharacterCapacity) {
            auto *ElementBuffer = CharacterAllocator.doAllocate(CharacterSize);
            Collections::doMove(CharacterContainer, CharacterSize, ElementBuffer);
            CharacterAllocator.doDeallocate(CharacterContainer, CharacterCapacity);
            CharacterContainer = CharacterAllocator.doAllocate(CharacterCapacity = CharacterCapacityNew);
            Collections::doMove(ElementBuffer, CharacterSize, CharacterContainer);
            CharacterAllocator.doDeallocate(ElementBuffer, CharacterSize);
        }
    }

    Reference<String> StringPool::doIntern(const String&) {
        throw NotImplementedException(u"StringPool::doIntern()"_S);
    }

    eLibraryAPI Character Literal::operator"" _C(char16_t CharacterSource) noexcept {
        return {CharacterSource};
    }

    eLibraryAPI CharacterLatin1 Literal::operator"" _CL1(char CharacterSource) noexcept {
        return {CharacterSource};
    }

    eLibraryAPI String Literal::operator"" _S(const char16_t *StringSource, size_t StringSize) {
        return {::std::u16string(StringSource, StringSize)};
    }
}
