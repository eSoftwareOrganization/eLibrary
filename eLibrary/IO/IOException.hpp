#pragma once

#ifndef eLibraryHeaderIOException
#define eLibraryHeaderIOException

#if eLibraryFeature(IO)

#include <Core/Exception.hpp>

namespace eLibrary::IO {
    using namespace eLibrary::Core;

    class IOException : public Exception {
    public:
        using Exception::Exception;
    };
}
#endif

#endif
