#pragma once

#include <IO/Exception.hpp>

namespace eLibrary::Network {
    class NetworkException final : public IO::IOException {
    public:
        explicit NetworkException(const String &ExceptionMessage) noexcept : IO::IOException(ExceptionMessage) {}
    };
}
