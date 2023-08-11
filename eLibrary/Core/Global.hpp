#pragma once

#include <cstdint>
#include <functional>
#include <utility>

namespace eLibrary::Core {
    template<typename T>
    concept Arithmetic = std::is_arithmetic<T>::value;

    template<typename T>
    concept Comparable = requires (const T &ObjectSource) {
        (intmax_t) ObjectSource.doCompare(ObjectSource);
    };

    template<typename T>
    concept Hashable = requires (const T &ObjectSource) {
        (uintmax_t) ObjectSource.hashCode();
    };

#define doDisableCopyAssignConstruct(ClassName) constexpr ClassName(const ClassName&) noexcept = delete;ClassName &operator=(const ClassName&) noexcept = delete;
#define doEnableCopyAssignConstruct(ClassName) ClassName(const ClassName &ObjectSource) {doAssign(ObjectSource);}ClassName &operator=(const ClassName &ObjectSource) {doAssign(ObjectSource);return *this;}
#define doEnableMoveAssignConstruct(ClassName) ClassName(ClassName &&ObjectSource) noexcept {doAssign(std::forward<ClassName>(ObjectSource));}ClassName &operator=(ClassName &&ObjectSource) noexcept {doAssign(std::forward<ClassName>(ObjectSource));return *this;}
#define eLibraryCompiler(CompilerName) eLibraryCompiler_##CompilerName
#define eLibraryFeature(FeatureName) eLibraryFeature_##FeatureName
#define eLibrarySystem(SystemName) eLibrarySystem_##SystemName

#define eLibraryAPI
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
