#pragma once

#ifndef eLibraryHeaderCoreGlobal
#define eLibraryHeaderCoreGlobal

#include <cstddef>
#include <cstdint>

namespace eLibrary::Core {
    class NonConstructable {
    public:
        constexpr NonConstructable() noexcept = delete;
    };

    class NonCopyAssignable {
    protected:
        constexpr NonCopyAssignable() noexcept = default;
    public:
        NonCopyAssignable &operator=(const NonCopyAssignable&) noexcept = delete;
    };

    class NonCopyConstructable {
    protected:
        constexpr NonCopyConstructable() noexcept = default;
    public:
        constexpr NonCopyConstructable(const NonCopyConstructable&) noexcept = delete;
    };

    class NonCopyable : public NonCopyAssignable, public NonCopyConstructable {};

    class NonMoveAssignable {
    protected:
        constexpr NonMoveAssignable() noexcept = default;
    public:
        NonMoveAssignable &operator=(NonMoveAssignable&&) noexcept = delete;
    };

    class NonMoveConstructable {
    protected:
        constexpr NonMoveConstructable() noexcept = default;
    public:
        constexpr NonMoveConstructable(NonMoveConstructable&&) noexcept = delete;
    };

    class NonMovable : public NonMoveAssignable, public NonMoveConstructable {};

#define doEnableCopyAssignConstruct(ClassName) ClassName(const ClassName &ObjectSource) {doAssign(ObjectSource);}ClassName &operator=(const ClassName &ObjectSource) {doAssign(ObjectSource);return *this;}
#define doEnableCopyAssignParameterConstruct(ClassName, ParameterName) ClassName(const ParameterName &ObjectSource) {doAssign(ObjectSource);}ClassName &operator=(const ParameterName &ObjectSource) {doAssign(ObjectSource);return *this;}
#define doEnableMoveAssignConstruct(ClassName) ClassName(ClassName &&ObjectSource) noexcept {doAssign(Objects::doMove(ObjectSource));}ClassName &operator=(ClassName &&ObjectSource) noexcept {doAssign(Objects::doMove(ObjectSource));return *this;}
#define doEnableMoveAssignParameterConstruct(ClassName, ParameterName) ClassName(ParameterName &&ObjectSource) noexcept {doAssign(Objects::doMove(ObjectSource));}ClassName &operator=(ParameterName &&ObjectSource) noexcept {doAssign(Objects::doMove(ObjectSource));return *this;}
#define doEnableValueAssignParameterConstruct(ClassName, ParameterName) ClassName(ParameterName ObjectSource) {doAssign(ObjectSource);}ClassName &operator=(ParameterName ObjectSource) {doAssign(ObjectSource);return *this;}
#define eLibraryArchitecture(ArchitectureName) eLibraryArchitecture_##ArchitectureName
#define eLibraryCompact(CompactName) eLibraryCompact_##CompactName
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
