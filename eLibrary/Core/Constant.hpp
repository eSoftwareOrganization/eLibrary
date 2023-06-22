#pragma once

#include <cstdint>
#include <functional>

namespace eLibrary::Core {
    template<typename T>
    concept Comparable = requires (const T &ObjectSource) {
        ObjectSource.doCompare(ObjectSource);
    };

    template<typename T>
    concept Hashable = requires (const T &ObjectSource) {
        ObjectSource.hashCode();
    };

    typedef unsigned char byte;
}

namespace std {
    template<eLibrary::Core::Comparable T>
    struct equal_to<T> {
    public:
        bool operator()(const T &Object1, const T &Object2) const noexcept {
            return !Object1.doCompare(Object2);
        }
    };

    template<eLibrary::Core::Comparable T>
    struct less<T> {
    public:
        bool operator()(const T &Object1, const T &Object2) const noexcept {
            return Object1.doCompare(Object2) < 0;
        }
    };

    template<eLibrary::Core::Comparable T>
    struct less_equal<T> {
    public:
        bool operator()(const T &Object1, const T &Object2) const noexcept {
            return Object1.doCompare(Object2) <= 0;
        }
    };

    template<eLibrary::Core::Comparable T>
    struct greater<T> {
    public:
        bool operator()(const T &Object1, const T &Object2) const noexcept {
            return Object1.doCompare(Object2) > 0;
        }
    };

    template<eLibrary::Core::Comparable T>
    struct greater_equal<T> {
    public:
        bool operator()(const T &Object1, const T &Object2) const noexcept {
            return Object1.doCompare(Object2) >= 0;
        }
    };

    template<eLibrary::Core::Hashable T>
    struct hash<T> {
    public:
        auto operator()(const T &ObjectSource) const noexcept {
            return ObjectSource.hashCode();
        }
    };

    template<eLibrary::Core::Comparable T>
    struct not_equal_to<T> {
    public:
        bool operator()(const T &Object1, const T &Object2) const noexcept {
            return Object1.doCompare(Object2);
        }
    };
}
