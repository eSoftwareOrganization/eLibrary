#pragma once

#if eLibraryFeature(Multimedia)

#include <IO/Exception.hpp>

namespace eLibrary::Multimedia {
    using namespace eLibrary::Core;

    class MediaException final : public IO::IOException {
    public:
        using IOException::IOException;

        const char *getClassName() const noexcept override {
            return "Multimedia::MediaException";
        }
    };
}
#endif
