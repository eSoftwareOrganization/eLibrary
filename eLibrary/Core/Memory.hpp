#pragma once

#include <Core/Exception.hpp>

namespace eLibrary::Core {
    class MemoryAllocator final : public Object {
    public:
        constexpr MemoryAllocator() noexcept = delete;

        template<typename T>
        static T *newArray(uintmax_t ArraySize) {
            return new T[ArraySize];
        }

        template<typename T, typename ...As>
        static T *newObject(As&&... ArgumentList) noexcept(std::is_nothrow_constructible<T, As...>::value) {
            return new T(std::forward<As>(ArgumentList)...);
        }
    };
}
