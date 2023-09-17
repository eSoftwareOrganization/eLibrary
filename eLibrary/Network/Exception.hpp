#pragma once

#if eLibraryFeature(Network)

#include <IO/Exception.hpp>

namespace eLibrary::Network {
    using namespace eLibrary::Core;

    class NetworkException final : public IO::IOException {
    public:
        doDefineClassMethod(NetworkException)

        using IOException::IOException;
    };
}
#endif
