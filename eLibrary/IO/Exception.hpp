#pragma once

#include <Core/Exception.hpp>

using namespace eLibrary::Core;

namespace eLibrary::IO {
    class IOException final : public Exception {
    public:
        explicit IOException(const String &ExceptionMessage) noexcept : Exception(ExceptionMessage) {}
    };
}
