#pragma once

#ifndef eLibraryHeaderNetworkException
#define eLibraryHeaderNetworkException

#if eLibraryFeature(Network)

#include <IO/IOException.hpp>

namespace eLibrary::Network {
    using namespace eLibrary::Core;

    class NetworkException final : public IO::IOException {
    public:
        using IOException::IOException;
    };
}
#endif

#endif
