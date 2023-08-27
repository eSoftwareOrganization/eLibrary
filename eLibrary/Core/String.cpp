#include <Core/Container.hpp>
#include <codecvt>
#include <locale>

namespace eLibrary::Core {
    uint8_t Character::toNumber(uint8_t NumberRadix) const {
        if (NumberRadix < 2 || NumberRadix > 36) throw Exception(String(u"Character::toNumber(uint8_t) NumberRadix"));
        if (isDigit()) {
            if (CharacterValue - 48 >= NumberRadix)
                throw Exception(String(u"Character::toNumber(uint8_t) NumberRadix"));
            return CharacterValue - 48;
        } else if (!isAlpha()) throw Exception(String(u"Character::toNumber(uint8_t) isAlpha"));
        if (towupper(CharacterValue) - 55 >= NumberRadix)
            throw Exception(String(u"Character::toNumber(uint8_t) NumberRadix"));
        return CharacterValue - 55;
    }

    String Character::toString() const noexcept {
        return *this;
    }

    Character Character::valueOf(uint8_t NumberSource, uint8_t NumberRadix) {
        if (NumberRadix < 2 || NumberRadix > 36) throw Exception(String(u"Character::valueOf(uint8_t, uint8_t) NumberRadix"));
        if (NumberSource >= NumberRadix) throw Exception(String(u"Character::valueOf(uint8_t, uint8_t) NumberSource"));
        if (NumberSource <= 10) return {char16_t(NumberSource + 48)};
        return {char16_t(NumberSource + 55)};
    }

    String::String(const Character &CharacterSource) noexcept {
        CharacterContainer = MemoryAllocator::newArray<Character>(2);
        CharacterContainer[0] = CharacterSource;
        CharacterContainer[1] = Character();
    }

    String::String(const std::string &StringSource) noexcept {
        std::u16string String16Source = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>().from_bytes(StringSource);
        CharacterContainer = MemoryAllocator::newArray<Character>((CharacterSize = (intmax_t) String16Source.size()) + 1);
        Arrays::doCopy(String16Source.begin(), String16Source.end(), CharacterContainer);
        CharacterContainer[CharacterSize] = Character();
    }

    String::String(const std::u16string &StringSource) noexcept : CharacterSize((intmax_t) StringSource.size()) {
        CharacterContainer = MemoryAllocator::newArray<Character>(StringSource.size() + 1);
        Arrays::doCopy(StringSource.begin(), StringSource.end(), CharacterContainer);
        CharacterContainer[CharacterSize] = Character();
    }

    String::String(const std::u32string &StringSource) noexcept {
        std::string String8Source(std::wstring_convert<std::codecvt_utf16<char32_t>, char32_t>().to_bytes(StringSource));
        std::u16string String16Source(reinterpret_cast<const char16_t*>(String8Source.c_str()), String8Source.size() / sizeof(char16_t));
        CharacterContainer = MemoryAllocator::newArray<Character>((CharacterSize = (intmax_t) String16Source.size()) + 1);
        Arrays::doCopy(String16Source.begin(), String16Source.end(), CharacterContainer);
        CharacterContainer[CharacterSize] = Character();
    }

    String::String(const std::wstring &StringSource) noexcept {
        if constexpr (sizeof(std::wstring::value_type) == sizeof(char16_t)) {
            CharacterContainer = MemoryAllocator::newArray<Character>((CharacterSize = (intmax_t) StringSource.size()) + 1);
            Arrays::doCopy(StringSource.begin(), StringSource.end(), CharacterContainer);
            CharacterContainer[CharacterSize] = Character();
        } else if constexpr (sizeof(std::wstring::value_type) == sizeof(char32_t)) {
            std::string String8Source(std::wstring_convert<std::codecvt_utf16<char32_t>, char32_t>().to_bytes({StringSource.begin(), StringSource.end()}));
            std::u16string String16Source(reinterpret_cast<const char16_t*>(String8Source.c_str()), String8Source.size() / sizeof(char16_t));
            CharacterContainer = MemoryAllocator::newArray<Character>((CharacterSize = (intmax_t) String16Source.size()) + 1);
            Arrays::doCopy(String16Source.begin(), String16Source.end(), CharacterContainer);
            CharacterContainer[CharacterSize] = Character();
        }
    }

    void String::doAssign(const String &StringSource) noexcept {
        if (Objects::getAddress(StringSource) == this) return;
        delete[] CharacterContainer;
        CharacterContainer = MemoryAllocator::newArray<Character>((CharacterSize = StringSource.CharacterSize) + 1);
        Arrays::doCopy(StringSource.CharacterContainer, CharacterSize, CharacterContainer);
        CharacterContainer[CharacterSize] = Character();
    }

    void String::doAssign(String &&StringSource) noexcept {
        if (Objects::getAddress(StringSource) == this) return;
        delete[] CharacterContainer;
        CharacterContainer = StringSource.CharacterContainer;
        CharacterSize = StringSource.CharacterSize;
        StringSource.CharacterContainer = nullptr;
        StringSource.CharacterSize = 0;
    }

    String String::doConcat(const Character &CharacterSource) const noexcept {
        StringStream CharacterStream(CharacterSize + 1);
        CharacterStream.addString(*this);
        CharacterStream.addCharacter(CharacterSource);
        return CharacterStream.toString();
    }

    String String::doConcat(const String &StringOther) const noexcept {
        StringStream CharacterStream(CharacterSize + StringOther.CharacterSize);
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
        StringStream CharacterStream(CharacterSize);
        for (intmax_t CharacterIndex = 0;CharacterIndex < CharacterSize;++CharacterIndex)
            CharacterStream.addCharacter(CharacterContainer[CharacterSize - CharacterIndex - 1]);
        return CharacterStream.toString();
    }

    String String::doStrip(const Character &CharacterSource) const noexcept {
        StringStream CharacterStream;
        for (intmax_t CharacterIndex = 0; CharacterIndex < CharacterSize; ++CharacterIndex)
            if (Objects::doCompare(CharacterContainer[CharacterIndex], CharacterSource)) CharacterStream.addCharacter(CharacterContainer[CharacterIndex]);
        return CharacterStream.toString();
    }

    String String::doStrip(const String &StringTarget) noexcept {
        StringStream CharacterStream;
        for (intmax_t Character1 = 0, Character2;Character1 < CharacterSize;++Character1) {
            for (Character2 = 0; Character2 < StringTarget.CharacterSize && !Objects::doCompare(CharacterContainer[Character1 + Character2], StringTarget.CharacterContainer[Character2]); ++Character2);
            if (Character2 == StringTarget.CharacterSize) Character1 += StringTarget.CharacterSize - 1;
            else CharacterStream.addCharacter(CharacterContainer[Character1]);
        }
        return CharacterStream.toString();
    }

    String String::doTruncate(intmax_t CharacterStart, intmax_t CharacterStop) const {
        if (CharacterStart < 0) CharacterStart += CharacterSize;
        Arrays::doCheckGE(CharacterStart, 0);
        Arrays::doCheckL(CharacterStart, CharacterSize);
        if (CharacterStop < 0) CharacterStop += CharacterSize + 1;
        Arrays::doCheckGE(CharacterStop, 0);
        Arrays::doCheckLE(CharacterStop, CharacterSize);
        Arrays::doCheckLE(CharacterStart, CharacterStop);
        StringStream CharacterStream(CharacterStop - CharacterStart);
        for (intmax_t CharacterIndex = CharacterStart;CharacterIndex < CharacterStop;++CharacterIndex)
            CharacterStream.addCharacter(CharacterContainer[CharacterIndex]);
        return CharacterStream.toString();
    }

    Character String::getCharacter(intmax_t CharacterIndex) const {
        if (CharacterIndex < 0) CharacterIndex += CharacterSize;
        Arrays::doCheckGE(CharacterIndex, 0);
        Arrays::doCheckL(CharacterIndex, CharacterSize);
        return CharacterContainer[CharacterIndex];
    }

    String String::toLowerCase() const noexcept {
        StringStream CharacterStream(CharacterSize);
        for (intmax_t CharacterIndex = 0;CharacterIndex < CharacterSize;++CharacterIndex)
            CharacterStream.addCharacter(CharacterContainer[CharacterIndex].toLowerCase());
        return CharacterStream.toString();
    }

    String String::toUpperCase() const noexcept {
        StringStream CharacterStream(CharacterSize);
        for (intmax_t CharacterIndex = 0;CharacterIndex < CharacterSize;++CharacterIndex)
            CharacterStream.addCharacter(CharacterContainer[CharacterIndex].toUpperCase());
        return CharacterStream.toString();
    }

    std::string String::toU8String() const noexcept {
        std::u16string StringSource(toU16String());
        return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>().to_bytes(StringSource.data(), StringSource.data() + StringSource.size());
    }

    std::u16string String::toU16String() const noexcept {
        auto *CharacterBuffer = MemoryAllocator::newArray<char16_t>(CharacterSize + 1);
        for (intmax_t CharacterIndex = 0;CharacterIndex < CharacterSize;++CharacterIndex)
            CharacterBuffer[CharacterIndex] = (char16_t) CharacterContainer[CharacterIndex];
        CharacterBuffer[CharacterSize] = char16_t();
        std::u16string CharacterResult(CharacterBuffer, (size_t) CharacterSize);
        delete[] CharacterBuffer;
        return CharacterResult;
    }

    std::u32string String::toU32String() const noexcept {
        std::u16string StringSource(toU16String());
        return std::wstring_convert<std::codecvt_utf16<char32_t>, char32_t>().from_bytes((const char*) StringSource.data(), (const char*) (StringSource.data() + StringSource.size()));
    }

    std::wstring String::toWString() const noexcept {
        if constexpr (sizeof(std::wstring::value_type) == sizeof(char16_t)) {
            std::u16string StringSource(toU16String());
            return {StringSource.begin(), StringSource.end()};
        } else if constexpr (sizeof(std::wstring::value_type) == sizeof(char32_t)) {
            std::u32string StringSource(toU32String());
            return {StringSource.begin(), StringSource.end()};
        }
    }

    StringStream::StringStream(uintmax_t CharacterCapacitySource) : CharacterCapacity(CharacterCapacitySource) {
        CharacterContainer = MemoryAllocator::newArray<Character>(CharacterCapacity);
    }

    void StringStream::addCharacter(const Character &CharacterSource) {
        if (!CharacterCapacity) CharacterContainer = MemoryAllocator::newArray<Character>(CharacterCapacity = 1);
        if (CharacterSize == CharacterCapacity) {
            auto *ElementBuffer = MemoryAllocator::newArray<Character>(CharacterSize);
            Arrays::doMove(CharacterContainer, CharacterSize, ElementBuffer);
            delete[] CharacterContainer;
            CharacterContainer = MemoryAllocator::newArray<Character>(CharacterCapacity <<= 1);
            Arrays::doMove(ElementBuffer, CharacterSize, CharacterContainer);
            delete[] ElementBuffer;
        }
        CharacterContainer[CharacterSize++] = CharacterSource;
    }

    void StringStream::addString(const String &StringSource) {
        if (!CharacterCapacity) CharacterContainer = MemoryAllocator::newArray<Character>(CharacterCapacity = 1);
        if (CharacterSize + StringSource.CharacterSize >= CharacterCapacity) {
            auto *ElementBuffer = MemoryAllocator::newArray<Character>(CharacterSize);
            Arrays::doMove(CharacterContainer, CharacterSize, ElementBuffer);
            delete[] CharacterContainer;
            while (CharacterCapacity <= CharacterSize + StringSource.CharacterSize) CharacterCapacity <<= 1;
            CharacterContainer = MemoryAllocator::newArray<Character>(CharacterCapacity);
            Arrays::doMove(ElementBuffer, CharacterSize, CharacterContainer);
            delete[] ElementBuffer;
        }
        Arrays::doCopy(StringSource.CharacterContainer, StringSource.CharacterSize, CharacterContainer + CharacterSize);
        CharacterSize += StringSource.CharacterSize;
    }

    String StringStream::toString() const noexcept {
        auto *CharacterBuffer = MemoryAllocator::newArray<char16_t>(CharacterSize + 1);
        for (uintmax_t CharacterIndex = 0;CharacterIndex < CharacterSize;++CharacterIndex)
            CharacterBuffer[CharacterIndex] = (char16_t) CharacterContainer[CharacterIndex];
        CharacterBuffer[CharacterSize] = char16_t();
        std::u16string CharacterResult(CharacterBuffer, (size_t) CharacterSize);
        delete[] CharacterBuffer;
        return CharacterResult;
    }
}
