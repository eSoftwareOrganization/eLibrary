#include <Core/Number.hpp>
#include <typeinfo>

#if __has_include(<cxxabi.h>)
#include <cxxabi.h>
#endif

namespace eLibrary::Core {
    String Object::toString() const noexcept {
        StringStream ObjectStream;
        uintmax_t ObjectHash = hashCode();
        while (ObjectHash) {
            if (ObjectHash % 16 < 10) ObjectStream.addCharacter(ObjectHash % 16 + 48);
            else ObjectStream.addCharacter(ObjectHash % 16 + 55);
            ObjectHash /= 16;
        }
        ObjectStream.addCharacter('x');
        ObjectStream.addCharacter('0');
        ObjectStream.addCharacter('@');
        return String(
#if eLibraryCompiler(GNU)
              abi::__cxa_demangle(typeid(*this).name(), nullptr, nullptr, nullptr)
#else
              typeid(*this).name()
#endif
        ).doConcat(ObjectStream.toString().doReverse());
    }

    template<typename K, typename V>
    V Objects::doMatchValue(const K &ObjectKeyTarget, std::initializer_list<ObjectEntry<K, V>> ObjectEntryList) {
        for (const auto &ObjectEntryCurrent : ObjectEntryList)
            if (!doCompare(ObjectEntryCurrent.EntryKey, ObjectKeyTarget)) return ObjectEntryCurrent.EntryValue;
        throw Exception(String(u"Objects::doMatchValue<K,V>(const K&, std::initializer_list<ObjectEntry<K, V>>)"));
    }
}
