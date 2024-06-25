#include <Core/String.hpp>

namespace eLibrary::Core {
    String Object::toString() const noexcept {
        StringBuilder ObjectStream;
        uintmax_t ObjectHash = hashCode();
        while (ObjectHash) {
            if (ObjectHash % 16 < 10) ObjectStream.addCharacter(ObjectHash % 16 + 48);
            else ObjectStream.addCharacter(ObjectHash % 16 + 55);
            ObjectHash /= 16;
        }
        ObjectStream.addCharacter('x');
        ObjectStream.addCharacter('0');
        ObjectStream.addCharacter('@');
        return (u"Object"_S).doConcat(ObjectStream.toString().doReverse());
    }
}
