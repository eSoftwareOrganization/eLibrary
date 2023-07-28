#pragma once

#if eLibraryFeature(IO)

#include <Core/Exception.hpp>

using namespace eLibrary::Core;

namespace eLibrary::IO {
    class IOException : public Exception {
    public:
        explicit IOException(const String &ExceptionMessage) noexcept : Exception(ExceptionMessage) {}
    };
}
#endif
