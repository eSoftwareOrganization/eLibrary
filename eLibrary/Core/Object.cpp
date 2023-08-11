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
}
