#pragma once

#if eLibraryFeature(Network)

#include <IO/Exception.hpp>

namespace eLibrary::Network {
    using namespace eLibrary::Core;

    class NetworkException final : public IO::IOException {
    public:
        using IOException::IOException;

        const char *getClassName() const noexcept override {
            return "Network::NetworkException";
        }
    };
}
#endif
