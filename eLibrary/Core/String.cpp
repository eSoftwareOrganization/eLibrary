#include <Core/Collection.hpp>
#include <Core/Number.hpp>

namespace eLibrary {
    char16_t String::getCharacter(intmax_t CharacterIndex) const {
        if (CharacterIndex < 0) CharacterIndex += CharacterSize;
        if (CharacterIndex < 0 || CharacterIndex >= CharacterSize)
            throw Exception(String(u"String::getCharacter(intmax_t) CharacterIndex"));
        return CharacterContainer[CharacterIndex];
    }

    String String::doConcat(char16_t CharacterSource) const noexcept {
        StringStream CharacterStream;
        CharacterStream.addString(CharacterContainer);
        CharacterStream.addCharacter(CharacterSource);
        return CharacterStream.toString();
    }

    String String::doConcat(const String &StringOther) const noexcept {
        StringStream CharacterStream;
        CharacterStream.addString(CharacterContainer);
        CharacterStream.addString(StringOther.CharacterContainer);
        return CharacterStream.toString();
    }

    String String::doReplace(const String &StringTarget, const String &StringSource) const noexcept {
        StringStream CharacterStream;
        std::u16string StringSource16(StringSource.toU16String());
        for (intmax_t Character1 = 0, Character2;Character1 < CharacterSize;++Character1) {
            for (Character2 = 0; Character2 < StringTarget.CharacterSize && CharacterContainer[Character1 + Character2] == StringTarget.CharacterContainer[Character2]; ++Character2);
            if (Character2 == StringTarget.CharacterSize) {
                CharacterStream.addString(StringSource16);
                Character1 += StringTarget.CharacterSize - 1;
            } else CharacterStream.addCharacter(CharacterContainer[Character1]);
        }
        return CharacterStream.toString();
    }

    String String::doReverse() const noexcept {
        StringStream CharacterStream;
        for (intmax_t CharacterIndex = 0;CharacterIndex < CharacterSize;++CharacterIndex)
            CharacterStream.addCharacter(CharacterContainer[CharacterSize - CharacterIndex - 1]);
        return CharacterStream.toString();
    }

    String String::doStrip(char16_t CharacterSource) const noexcept {
        StringStream CharacterStream;
        for (intmax_t CharacterIndex = 0; CharacterIndex < CharacterSize; ++CharacterIndex)
            if (CharacterContainer[CharacterIndex] != CharacterSource) CharacterStream.addCharacter(CharacterContainer[CharacterIndex]);
        return CharacterStream.toString();
    }

    String String::doStrip(const String &StringTarget) noexcept {
        StringStream CharacterStream;
        for (intmax_t Character1 = 0, Character2;Character1 < CharacterSize;++Character1) {
            for (Character2 = 0; Character2 < StringTarget.CharacterSize && CharacterContainer[Character1 + Character2] == StringTarget.CharacterContainer[Character2]; ++Character2);
            if (Character2 == StringTarget.CharacterSize) Character1 += StringTarget.CharacterSize - 1;
            else CharacterStream.addCharacter(CharacterContainer[Character1]);
        }
        return CharacterStream.toString();
    }

    String String::doTruncate(intmax_t CharacterStart, intmax_t CharacterStop) const {
        if (CharacterStart < 0) CharacterStart += CharacterSize;
        if (CharacterStop < 0) CharacterStop += CharacterSize + 1;
        if (CharacterStart < 0 || CharacterStart >= CharacterSize)
            throw Exception(String(u"String::doTruncate(intmax_t, intmax_t) CharacterStart"));
        if (CharacterStop < 0 || CharacterStop > CharacterSize)
            throw Exception(String(u"String::doTruncate(intmax_t, intmax_t) CharacterStop"));
        if (CharacterStop < CharacterStart)
            throw Exception(String(u"String::doTruncate(intmax_t, intmax_t) CharacterStart CharacterStop"));
        StringStream CharacterStream;
        for (intmax_t CharacterIndex = CharacterStart;CharacterIndex < CharacterStop;++CharacterIndex)
            CharacterStream.addCharacter(CharacterContainer[CharacterIndex]);
        return CharacterStream.toString();
    }

    String String::toLowerCase() const noexcept {
        StringStream CharacterStream;
        for (intmax_t CharacterIndex = 0;CharacterIndex < CharacterSize;++CharacterIndex)
            CharacterStream.addCharacter(towlower(CharacterContainer[CharacterIndex]));
        return CharacterStream.toString();
    }

    String String::toUpperCase() const noexcept {
        StringStream CharacterStream;
        for (intmax_t CharacterIndex = 0;CharacterIndex < CharacterSize;++CharacterIndex)
            CharacterStream.addCharacter(towupper(CharacterContainer[CharacterIndex]));
        return CharacterStream.toString();
    }
}
