#include <Core/Number.hpp>

#ifdef __GNUC__
#include <cxxabi.h>
#endif

namespace eLibrary {
    String Object::toString() const noexcept {
        StringStream ObjectStream;
        ObjectStream.addString(String(
#ifdef __GNUC__
            abi::__cxa_demangle(typeid(*this).name(), nullptr, nullptr, nullptr)
#else
            typeid(*this).name()
#endif
        ).toU16String());
        ObjectStream.addString(u"@0x");
        ObjectStream.addString(Integer(hashCode()).toString(16).toU16String());
        return ObjectStream.toString();
    }
}
