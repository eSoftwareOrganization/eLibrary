#pragma once

#ifndef eLibraryHeaderCoreGlobal
#define eLibraryHeaderCoreGlobal

#include <concepts>
#include <cstdint>
#include <type_traits>

namespace eLibrary::Core {
    template<typename T>
    concept Arithmetic = ::std::is_arithmetic_v<T>;

    template<typename T>
    concept Comparable = requires (const T &ObjectSource) {
        {ObjectSource.doCompare(ObjectSource)} -> ::std::same_as<intmax_t>;
    };

    template<typename T>
    concept Hashable = requires (const T &ObjectSource) {
        {ObjectSource.hashCode()} -> ::std::same_as<uintmax_t>;
    };

    class NonCopyable {
    protected:
        constexpr NonCopyable() noexcept = default;
    public:
        constexpr NonCopyable(const NonCopyable&) noexcept = delete;

        NonCopyable &operator=(const NonCopyable&) noexcept = delete;
    };

    template<typename T1, typename T2>
    concept SizeEqual = sizeof(T1) == sizeof(T2);

    template<typename T>
    concept StringConvertible = requires (const T &ObjectSource) {
        ObjectSource.toString();
    };

#define doEnableCopyAssignConstruct(ClassName) ClassName(const ClassName &ObjectSource) {doAssign(ObjectSource);}ClassName &operator=(const ClassName &ObjectSource) {doAssign(ObjectSource);return *this;}
#define doEnableCopyAssignParameterConstruct(ClassName, ParameterName) ClassName(const ParameterName &ObjectSource) {doAssign(ObjectSource);}ClassName &operator=(const ParameterName &ObjectSource) {doAssign(ObjectSource);return *this;}
#define doEnableMoveAssignConstruct(ClassName) ClassName(ClassName &&ObjectSource) noexcept {doAssign(Objects::doMove(ObjectSource));}ClassName &operator=(ClassName &&ObjectSource) noexcept {doAssign(Objects::doMove(ObjectSource));return *this;}
#define doEnableMoveAssignParameterConstruct(ClassName, ParameterName) ClassName(ParameterName &&ObjectSource) noexcept {doAssign(Objects::doMove(ObjectSource));}ClassName &operator=(ParameterName &&ObjectSource) noexcept {doAssign(Objects::doMove(ObjectSource));return *this;}
#define eLibraryArchitecture(ArchitectureName) eLibraryArchitecture_##ArchitectureName
#define eLibraryCompiler(CompilerName) eLibraryCompiler_##CompilerName
#define eLibraryFeature(FeatureName) eLibraryFeature_##FeatureName
#define eLibraryLinkage(LinkageName) eLibraryLinkage_##LinkageName
#define eLibrarySystem(SystemName) eLibrarySystem_##SystemName

#if eLibraryLinkage(Static)
#define eLibraryAPI
#else
#if eLibrarySystem(Windows)
#define eLibraryAPI
#else
#define eLibraryAPI __attribute__((visibility("default")))
#endif
#endif
}

#endif
