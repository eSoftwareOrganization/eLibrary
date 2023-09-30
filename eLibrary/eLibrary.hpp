#pragma once

#include <Core/Global.hpp>

#if eLibrarySystem(Windows)
#include <ws2tcpip.h>
#include <Windows.h>
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
    static void doInitializeCore() {
#define doDefineType(TypeMode, TypeName, ...) auto *TypeName##TypeMode(Core::MemoryAllocator::newObject<Core::Type##TypeMode>(#TypeName, ##__VA_ARGS__));
#define doDefineRegisterType(TypeMode, TypeName, ...) doDefineType(TypeMode, TypeName, ##__VA_ARGS__)Core::TypeManager::getInstance()->doRegister##TypeMode(TypeName##TypeMode)
#define doDefineRegisterClass(TypeName, ...) doDefineRegisterType(Class, TypeName, ##__VA_ARGS__)

        doDefineRegisterClass(Array, "Object");
        doDefineRegisterClass(ArrayList, "Object");
        doDefineRegisterClass(ArraySet, "Object");
        doDefineRegisterClass(ContainerQueue, "Object");
        doDefineRegisterClass(ContainerStack, "Object");
        doDefineRegisterClass(DoubleLinkedList, "Object");
        doDefineRegisterClass(DoubleLinkedSet, "Object");
        doDefineRegisterClass(Fraction, "Object");
        doDefineRegisterClass(Integer, "Object");
        doDefineRegisterClass(Object, "");
        doDefineRegisterClass(SingleLinkedList, "Object");
        doDefineRegisterClass(SingleLinkedSet, "Object");
        doDefineRegisterClass(String, "Object");
        doDefineRegisterClass(Thread, "Object");
    }
}
