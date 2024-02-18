#pragma once

#ifndef eLibraryHeadereLibrary
#define eLibraryHeadereLibrary

#define eLibraryToStringImplement(ObjectSource) #ObjectSource
#define eLibraryToString(ObjectSource) eLibraryToStringImplement(ObjectSource)

#define eLibraryVersionMajor 0
#define eLibraryVersionMinor 20
#define eLibraryVersionPatch 4
#define eLibraryVersionNumber (eLibraryVersionMajor * 10000 + eLibraryVersionMinor * 100 + eLibraryVersionPatch)
#define eLibraryVersionString (eLibraryToString(eLibraryVersionMajor) "." eLibraryToString(eLibraryVersionMinor) "." eLibraryToString(eLibraryVersionPatch))

#include <Core/Global.hpp>

#if eLibrarySystem(Windows)
#define WIN32_LEAN_AND_MEAN
#endif

#include <Core/Concurrent.hpp>
#include <Core/Container.hpp>
#include <Core/Mathematics.hpp>

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

#if eLibraryFeature(ExtensionGeoIP)
#include <Extension/Extension-GeoIP.hpp>
#endif

#if eLibrarySystem(Windows)
#include <Platform/Windows.hpp>
#endif

#endif
