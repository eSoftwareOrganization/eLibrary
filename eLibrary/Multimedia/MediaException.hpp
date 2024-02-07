#pragma once

#ifndef eLibraryHeaderMultimediaException
#define eLibraryHeaderMultimediaException

#if eLibraryFeature(Multimedia)

#include <IO/IOException.hpp>

namespace eLibrary::Multimedia {
    using namespace eLibrary::Core;

    class MediaException final : public IO::IOException {
    public:
        using IOException::IOException;
    };
}
#endif

#endif
