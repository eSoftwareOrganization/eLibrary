#pragma once

#include <Core/Global.hpp>
#include <functional>
#include <utility>

namespace eLibrary::Core {
    class Class;
    class String;

    class Object {
    public:
        virtual ~Object() noexcept = default;

        virtual uintmax_t hashCode() const noexcept {
            return (uintmax_t) this;
        }

        eLibraryAPI virtual String toString() const noexcept;
    };

    template<typename T>
    concept ObjectDerived = std::derived_from<T, Object>;

    class Objects final : public Object {
    public:
        constexpr Objects() noexcept = delete;

        template<typename F, typename ...Fs>
        static void doCall(F FunctionCurrent, Fs ...FunctionList) {
            FunctionCurrent();
            if constexpr (sizeof...(Fs)) doCall(FunctionList...);
        }

        template<std::integral T>
        static intmax_t doCompare(T Object1, T Object2) noexcept {
            if (Object1 > Object2) return 1;
            if (Object1 < Object2) return -1;
            return 0;
        }

        template<Comparable T>
        static intmax_t doCompare(const T &Object1, const T &Object2) {
            return Object1.doCompare(Object2);
        }

        template<Comparable T>
        static intmax_t doCompare(const T *Object1, const T *Object2) {
            return Object1->doCompare(*Object2);
        }

        template<typename T>
        static constexpr T &&doForward(std::remove_reference_t<T> &ObjectSource) noexcept {
            return static_cast<T&&>(ObjectSource);
        }

        template<typename T>
        static constexpr T &&doForward(std::remove_reference_t<T> &&ObjectSource) noexcept {
            return static_cast<T&&>(ObjectSource);
        }

        template<typename T>
        static constexpr std::remove_reference_t<T>&& doMove(T &&ObjectSource) noexcept {
            return static_cast<std::remove_reference_t<T>&&>(ObjectSource);
        }

        template<typename T>
        static void doSwap(T &Object1, T &Object2) noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_assignable_v<T>) {
            T ObjectBuffer = doMove(Object1);
            Object1 = doMove(Object2);
            Object2 = doMove(ObjectBuffer);
        }

        template<typename T> requires std::is_object_v<T>
        static T *getAddress(T &ObjectSource) noexcept {
            return (T*) &((char&) ObjectSource);
        }

        template<typename T> requires (!std::is_object_v<T>)
        static T *getAddress(T &ObjectSource) noexcept {
            return &ObjectSource;
        }

        template<Arithmetic T1, Arithmetic T2>
        static auto getMaximum(T1 Object1, T2 Object2) noexcept {
            return Object1 >= Object2 ? Object1 : Object2;
        }

        template<Comparable T>
        static T getMaximum(const T &Object1, const T &Object2) {
            return doCompare(Object1, Object2) >= 0 ? Object1 : Object2;
        }

        template<Arithmetic T1, Arithmetic T2>
        static auto getMinimum(T1 Object1, T2 Object2) noexcept {
            return Object1 <= Object2 ? Object1 : Object2;
        }

        template<Comparable T>
        static T getMinimum(const T &Object1, const T &Object2) {
            return doCompare(Object1, Object2) <= 0 ? Object1 : Object2;
        }
    };
}

template<eLibrary::Core::Comparable T>
struct std::equal_to<T> {
public:
    bool operator()(const T &Object1, const T &Object2) const noexcept {
        return !Object1.doCompare(Object2);
    }
};

template<eLibrary::Core::Comparable T>
struct std::greater<T> {
public:
    bool operator()(const T &Object1, const T &Object2) const noexcept {
        return Object1.doCompare(Object2) > 0;
    }
};

template<eLibrary::Core::Comparable T>
struct std::greater_equal<T> {
public:
    bool operator()(const T &Object1, const T &Object2) const noexcept {
        return Object1.doCompare(Object2) >= 0;
    }
};

template<eLibrary::Core::Hashable T>
struct std::hash<T> {
public:
    auto operator()(const T &ObjectSource) const noexcept {
        return ObjectSource.hashCode();
    }
};

template<eLibrary::Core::Comparable T>
struct std::less<T> {
public:
    bool operator()(const T &Object1, const T &Object2) const noexcept {
        return Object1.doCompare(Object2) < 0;
    }
};

template<eLibrary::Core::Comparable T>
struct std::less_equal<T> {
public:
    bool operator()(const T &Object1, const T &Object2) const noexcept {
        return Object1.doCompare(Object2) <= 0;
    }
};

template<eLibrary::Core::Comparable T>
struct std::not_equal_to<T> {
public:
    bool operator()(const T &Object1, const T &Object2) const noexcept {
        return Object1.doCompare(Object2);
    }
};
