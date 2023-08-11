#pragma once

#include <Core/Exception.hpp>
#include <new>

namespace eLibrary::Core {
    class MemoryAllocator final : public Object {
    public:
        constexpr MemoryAllocator() noexcept = delete;

        template<typename T>
        static T *newArray(uintmax_t ArraySize) {
            return new (std::nothrow) T[ArraySize];
        }

        template<typename T, typename ...As>
        static T *newObject(As&&... ArgumentList) noexcept(std::is_nothrow_constructible<T, As...>::value) {
            return new (std::nothrow) T(Objects::doForward<As>(ArgumentList)...);
        }
    };
}
