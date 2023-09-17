#pragma once

#if eLibraryFeature(IO)

#include <Core/Exception.hpp>

namespace eLibrary::IO {
    using namespace eLibrary::Core;

    class IOException : public Exception {
    public:
        doDefineClassMethod(IOException)

        using Exception::Exception;
    };
}
#endif
