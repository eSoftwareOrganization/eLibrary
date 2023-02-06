#include <Core/String.hpp>
#include <sstream>

#ifdef __GNUC__
#include <cxxabi.h>
#endif

namespace eLibrary {
    String Object::toString() const noexcept {
        std::stringstream ObjectStream;
        ObjectStream <<
#ifdef __GNUC__
            abi::__cxa_demangle(typeid(*this).name(), nullptr, nullptr, nullptr)
#else
            typeid(*this).name()
#endif
        << "@0x" << std::hex << std::uppercase << hashCode();
        return ObjectStream.str();
    }
}
