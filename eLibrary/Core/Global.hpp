#pragma once

#include <concepts>
#include <cstdint>
#include <type_traits>

namespace eLibrary::Core {
    template<typename T>
    concept Arithmetic = std::is_arithmetic_v<T>;

    template<typename T>
    concept Comparable = requires (const T &ObjectSource) {
        {ObjectSource.doCompare(ObjectSource)} -> std::same_as<intmax_t>;
    };

    template<typename T>
    concept Hashable = requires (const T &ObjectSource) {
        {ObjectSource.hashCode()} -> std::same_as<uintmax_t>;
    };

#define doDisableCopyAssignConstruct(ClassName) constexpr ClassName(const ClassName&) noexcept = delete;ClassName &operator=(const ClassName&) noexcept = delete;
#define doEnableCopyAssignConstruct(ClassName) ClassName(const ClassName &ObjectSource) {doAssign(ObjectSource);}ClassName &operator=(const ClassName &ObjectSource) {doAssign(ObjectSource);return *this;}
#define doEnableCopyAssignParameterConstruct(ClassName, ParameterName) ClassName(const ParameterName &ObjectSource) {doAssign(ObjectSource);}ClassName &operator=(const ParameterName &ObjectSource) {doAssign(ObjectSource);return *this;}
#define doEnableMoveAssignConstruct(ClassName) ClassName(ClassName &&ObjectSource) noexcept {doAssign(Objects::doMove(ObjectSource));}ClassName &operator=(ClassName &&ObjectSource) noexcept {doAssign(Objects::doMove(ObjectSource));return *this;}
#define doEnableMoveAssignParameterConstruct(ClassName, ParameterName) ClassName(ParameterName &&ObjectSource) noexcept {doAssign(Objects::doMove(ObjectSource));}ClassName &operator=(ParameterName &&ObjectSource) noexcept {doAssign(Objects::doMove(ObjectSource));return *this;}
#define eLibraryCompiler(CompilerName) eLibraryCompiler_##CompilerName
#define eLibraryFeature(FeatureName) eLibraryFeature_##FeatureName
#define eLibrarySystem(SystemName) eLibrarySystem_##SystemName

#define eLibraryAPI
}
