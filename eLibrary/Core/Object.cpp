#include <Core/Number.hpp>

#ifdef __GNUC__
#include <cxxabi.h>
#endif

namespace eLibrary::Core {
    String Object::toString() const noexcept {
        StringStream ObjectStream;
        ObjectStream.addString({
#ifdef eLibraryCompilerGNU
            abi::__cxa_demangle(typeid(*this).name(), nullptr, nullptr, nullptr)
#else
            typeid(*this).name()
#endif
        });
        ObjectStream.addString({u"@0x"});
        ObjectStream.addString(Integer(hashCode()).toString(16));
        return ObjectStream.toString();
    }
}
