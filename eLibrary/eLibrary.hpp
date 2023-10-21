#pragma once

#ifndef eLibraryHeadereLibrary
#define eLibraryHeadereLibrary

#define eLibraryToStringImplement(ObjectSource) #ObjectSource
#define eLibraryToString(ObjectSource) eLibraryToStringImplement(ObjectSource)

#define eLibraryVersionMajor 2023
#define eLibraryVersionMinor 18
#define eLibraryVersionPatch 0
#define eLibraryVersionNumber (eLibraryVersionMajor * 10000 + eLibraryVersionMinor * 100 + eLibraryVersionPatch)
#define eLibraryVersionString (eLibraryToString(eLibraryVersionMajor) "." eLibraryToString(eLibraryVersionMinor) "." eLibraryToString(eLibraryVersionPatch))

#include <Core/Global.hpp>

#if eLibrarySystem(Windows)
#define WIN32_LEAN_AND_MEAN
#endif

#include <Core/Concurrent.hpp>
#include <Core/Container.hpp>
#include <Core/Mathematics.hpp>
#include <Core/Type.hpp>

#if eLibraryFeature(IO)
#include <IO/Buffer.hpp>
#include <IO/FileStream.hpp>
#include <IO/Library.hpp>
#endif

#if eLibraryFeature(Multimedia)
#include <Multimedia/AudioSegment.hpp>
#endif

#if eLibraryFeature(Network)
#include <Network/Socket.hpp>
#endif

namespace eLibrary {
#define doDefineType(TypeMode, TypeName, ...) auto *TypeName##Type(Core::MemoryAllocator::newObject<Core::Type##TypeMode>(#TypeName, ##__VA_ARGS__));
#define doDefineRegisterType(TypeMode, TypeName, ...) doDefineType(TypeMode, TypeName, ##__VA_ARGS__)Core::TypeManager::getInstance()->doRegisterType(TypeName##Type)
#define doDefineRegisterClass(TypeName, ...) doDefineRegisterType(Class, TypeName, ##__VA_ARGS__)

    namespace Core {
        static void doInitializeCore() {
            doDefineRegisterClass(Array, "Object");
            doDefineRegisterClass(ArrayList, "Object");
            doDefineRegisterClass(ArraySet, "Object");
            doDefineRegisterClass(ContainerQueue, "Object");
            doDefineRegisterClass(ContainerStack, "Object");
            doDefineRegisterClass(DoubleLinkedList, "Object");
            doDefineRegisterClass(DoubleLinkedSet, "Object");
            doDefineRegisterClass(Fraction, "Object");
            doDefineRegisterClass(Integer, "Object");
            doDefineRegisterClass(NumberBuiltin, "Object");
            doDefineRegisterClass(Object, "");
            doDefineRegisterClass(SingleLinkedList, "Object");
            doDefineRegisterClass(SingleLinkedSet, "Object");
            doDefineRegisterClass(String, "Object");
            doDefineRegisterClass(Thread, "Object");
        }
    }
}

#endif
