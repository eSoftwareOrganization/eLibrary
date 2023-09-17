#include <Core/String.hpp>

namespace eLibrary::Core {
    Class Object::getClass() const noexcept {
        doDefineClass(Object)
        return ObjectClass;
    }

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
        return String(getClass().getClassName()).doConcat(ObjectStream.toString().doReverse());
    }
}
