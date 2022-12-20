#include <Core/Collection.hpp>

namespace eLibrary {
    String::String(char16_t *StringSource, intmax_t StringSize) : CharacterSize(StringSize), CharacterContainer(StringSource) {
        if (!StringSource) throw Exception(String(u"String::String(char16_t*, intmax_t) StringSource"));
        CharacterReference = new uintmax_t;
        *CharacterReference = 1;
    }

    String::String(char16_t *StringSource, intmax_t StringSize, bool StringRecycle) {
        if (!StringSource) throw Exception(String(u"String::String(char16_t*, intmax_t, bool) StringSource"));
        for (CharacterSize = 0; StringSource[CharacterSize]; ++CharacterSize);
        if (StringSize > CharacterSize) throw Exception(String(u"String::String(char16_t*, intmax_t, bool) StringSize"));
        CharacterContainer = new char16_t[CharacterSize + 1];
        memcpy(CharacterContainer, StringSource, sizeof(char16_t) * CharacterSize);
        CharacterContainer[CharacterSize] = char16_t();
        if (StringRecycle) delete[] StringSource;
        CharacterReference = new uintmax_t;
        *CharacterReference = 1;
    }

    char16_t String::getCharacter(intmax_t CharacterIndex) const {
        if (CharacterIndex < 0) CharacterIndex += CharacterSize;
        if (CharacterIndex < 0 || CharacterIndex >= CharacterSize)
            throw Exception(String(u"String::getCharacter(intmax_t) CharacterIndex"));
        return CharacterContainer[CharacterIndex];
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
        auto *StringBuffer = new char16_t[CharacterStop - CharacterStart + 1];
        memcpy(StringBuffer, CharacterContainer + CharacterStart, sizeof(char16_t) * (CharacterStop - CharacterStart));
        StringBuffer[CharacterStop - CharacterStart] = char16_t();
        return String(StringBuffer, CharacterStop - CharacterStart, true);
    }
}
