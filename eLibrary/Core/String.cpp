#include <Core/Collection.hpp>

namespace eLibrary {
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
        std::basic_stringstream<char16_t> CharacterStream;
        for (intmax_t CharacterIndex = CharacterStart;CharacterIndex < CharacterStop;++CharacterIndex)
            CharacterStream << CharacterContainer[CharacterIndex];
        return CharacterStream.str();
    }
}
