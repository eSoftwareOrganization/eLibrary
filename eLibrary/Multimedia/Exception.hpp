#pragma once

#if eLibraryFeature(Multimedia)

#include <IO/Exception.hpp>

namespace eLibrary::Multimedia {
    class MediaException final : public IO::IOException {
    public:
        explicit MediaException(const String &ExceptionMessage) noexcept : IO::IOException(ExceptionMessage) {}
    };
}
#endif
