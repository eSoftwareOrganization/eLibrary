#pragma once

#if eLibraryFeature(Multimedia)

#include <IO/Exception.hpp>

namespace eLibrary::Multimedia {
    using namespace eLibrary::Core;

    class MediaException final : public IO::IOException {
    public:
        doDefineClassMethod(MediaException)

        using IOException::IOException;
    };
}
#endif
