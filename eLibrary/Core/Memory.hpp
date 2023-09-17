#pragma once

#include <Core/Exception.hpp>
#include <new>

namespace eLibrary::Core {
    class MemoryAllocator final : public Object {
    public:
        constexpr MemoryAllocator() noexcept = delete;

        template<typename T>
        static void deleteArray(T *ArraySource) noexcept {
            delete[] ArraySource;
        }

        template<typename T>
        static void deleteObject(T *ObjectSource) noexcept(std::is_nothrow_destructible_v<T>) {
            delete ObjectSource;
        }

        template<typename T>
        static T *newArray(uintmax_t ArraySize) noexcept {
            return new (std::nothrow) T[ArraySize];
        }

        template<typename T, typename ...As>
        static T *newObject(As&&... ArgumentList) noexcept(std::is_nothrow_constructible<T, As...>::value) {
            return new (std::nothrow) T(Objects::doForward<As>(ArgumentList)...);
        }
    };
}
