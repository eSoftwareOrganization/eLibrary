#include <Core/Memory.hpp>

namespace eLibrary::Core {
    String::String(const eLibrary::Core::Character &CharacterSource) {
        CharacterContainer = MemoryAllocator::newArray<Character>(2);
        CharacterContainer[0] = CharacterSource;
        CharacterContainer[1] = Character();
    }

    String::String(const std::string &StringSource) : CharacterSize((intmax_t) StringSource.size()) {
        static std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> StringConverter;
        std::u16string String16Source = StringConverter.from_bytes(StringSource);
        CharacterContainer = MemoryAllocator::newArray<Character>(CharacterSize + 1);
        std::copy(String16Source.begin(), String16Source.end(), CharacterContainer);
        CharacterContainer[CharacterSize] = Character();
    }

    String::String(const std::u16string &StringSource) : CharacterSize((intmax_t) StringSource.size()) {
        CharacterContainer = new Character[StringSource.size() + 1];
        std::copy(StringSource.begin(), StringSource.end(), CharacterContainer);
        CharacterContainer[CharacterSize] = Character();
    }

    String::String(const std::u32string &StringSource) : CharacterSize((intmax_t) StringSource.size()) {
        static std::wstring_convert<std::codecvt_utf16<char32_t>, char32_t> StringConverter;
        std::string String8Source = StringConverter.to_bytes(StringSource);
        std::u16string String16Source(reinterpret_cast<const char16_t*>(String8Source.c_str()), String8Source.size() / sizeof(char16_t));
        CharacterContainer = MemoryAllocator::newArray<Character>(String16Source.size() + 1);
        std::copy(String16Source.begin(), String16Source.end(), CharacterContainer);
        CharacterContainer[CharacterSize] = Character();
    }

    String::String(const std::wstring &StringSource) : CharacterSize((intmax_t) StringSource.size()) {
        CharacterContainer = MemoryAllocator::newArray<Character>(CharacterSize + 1);
        std::copy(StringSource.begin(), StringSource.end(), CharacterContainer);
        CharacterContainer[CharacterSize] = Character();
    }

    void String::doAssign(const String &StringSource) {
        if (Objects::getAddress(StringSource) == this) return;
        delete[] CharacterContainer;
        CharacterContainer = new Character[(CharacterSize = StringSource.CharacterSize) + 1];
        std::copy_n(StringSource.CharacterContainer, CharacterSize, CharacterContainer);
        CharacterContainer[CharacterSize] = Character();
    }

    String String::doConcat(const Character &CharacterSource) const noexcept {
        StringStream CharacterStream;
        CharacterStream.addString(*this);
        CharacterStream.addCharacter(CharacterSource);
        return CharacterStream.toString();
    }

    String String::doConcat(const String &StringOther) const noexcept {
        StringStream CharacterStream;
        CharacterStream.addString(*this);
        CharacterStream.addString(StringOther);
        return CharacterStream.toString();
    }

    String String::doReplace(const String &StringTarget, const String &StringSource) const noexcept {
        StringStream CharacterStream;
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
        StringStream CharacterStream;
        for (intmax_t CharacterIndex = 0;CharacterIndex < CharacterSize;++CharacterIndex)
            CharacterStream.addCharacter(CharacterContainer[CharacterSize - CharacterIndex - 1]);
        return CharacterStream.toString();
    }

    String String::doStrip(const Character &CharacterSource) const noexcept {
        StringStream CharacterStream;
        for (intmax_t CharacterIndex = 0; CharacterIndex < CharacterSize; ++CharacterIndex)
            if (CharacterContainer[CharacterIndex].doCompare(CharacterSource)) CharacterStream.addCharacter(CharacterContainer[CharacterIndex]);
        return CharacterStream.toString();
    }

    String String::doStrip(const String &StringTarget) noexcept {
        StringStream CharacterStream;
        for (intmax_t Character1 = 0, Character2;Character1 < CharacterSize;++Character1) {
            for (Character2 = 0; Character2 < StringTarget.CharacterSize && !CharacterContainer[Character1 + Character2].doCompare(StringTarget.CharacterContainer[Character2]); ++Character2);
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

    Character String::getCharacter(intmax_t CharacterIndex) const {
        if (CharacterIndex < 0) CharacterIndex += CharacterSize;
        if (CharacterIndex < 0 || CharacterIndex >= CharacterSize)
            throw Exception(String(u"String::getCharacter(intmax_t) CharacterIndex"));
        return CharacterContainer[CharacterIndex];
    }

    String String::toLowerCase() const noexcept {
        StringStream CharacterStream;
        for (intmax_t CharacterIndex = 0;CharacterIndex < CharacterSize;++CharacterIndex)
            CharacterStream.addCharacter(CharacterContainer[CharacterIndex].toLowerCase());
        return CharacterStream.toString();
    }

    String String::toUpperCase() const noexcept {
        StringStream CharacterStream;
        for (intmax_t CharacterIndex = 0;CharacterIndex < CharacterSize;++CharacterIndex)
            CharacterStream.addCharacter(CharacterContainer[CharacterIndex].toUpperCase());
        return CharacterStream.toString();
    }

    void StringStream::addCharacter(const Character &CharacterSource) {
        if (!CharacterCapacity) CharacterContainer = MemoryAllocator::newArray<Character>(CharacterCapacity = 1);
        if (CharacterSize == CharacterCapacity) {
            auto *ElementBuffer = MemoryAllocator::newArray<Character>(CharacterSize);
            std::copy_n(CharacterContainer, CharacterSize, ElementBuffer);
            delete[] CharacterContainer;
            CharacterContainer = MemoryAllocator::newArray<Character>(CharacterCapacity <<= 1);
            std::copy_n(ElementBuffer, CharacterSize, CharacterContainer);
            delete[] ElementBuffer;
        }
        CharacterContainer[CharacterSize++] = CharacterSource;
    }
}
