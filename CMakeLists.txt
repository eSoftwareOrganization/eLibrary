cmake_minimum_required(VERSION 3.17)

cmake_policy(SET CMP0091 NEW)

option(ELIBRARY_FEATURE_ARCHIVE "eLibrary::IO::Archive" OFF)
option(ELIBRARY_MODULE_IO "eLibrary::IO" ON)
option(ELIBRARY_MODULE_MULTIMEDIA "eLibrary::Multimedia" OFF)
option(ELIBRARY_MODULE_NETWORK "eLibrary::Network" OFF)
option(ELIBRARY_UNIT_TEST "eLibrary::eLibraryTest" OFF)

if(ELIBRARY_FEATURE_ARCHIVE)
    list(APPEND VCPKG_MANIFEST_FEATURES "feature-archive")
    add_compile_definitions(eLibraryFeature_Archive=1)
endif()

if(ELIBRARY_MODULE_IO)
    add_compile_definitions(eLibraryFeature_IO=1)
endif()

if(ELIBRARY_MODULE_MULTIMEDIA)
    list(APPEND VCPKG_MANIFEST_FEATURES "module-multimedia")
    add_compile_definitions(eLibraryFeature_Multimedia=1)
endif()

if(ELIBRARY_MODULE_NETWORK)
    list(APPEND VCPKG_MANIFEST_FEATURES "module-network")
    add_compile_definitions(eLibraryFeature_Network=1)
endif()

if(ELIBRARY_UNIT_TEST)
    list(APPEND VCPKG_MANIFEST_FEATURES "application-test")
endif()

project(eLibrary DESCRIPTION "An all-in-one cpp library")

set(CMAKE_CXX_STANDARD 23)

include_directories(eLibrary)

add_compile_options(-Wall)

add_compile_definitions(eLibraryCompiler_${CMAKE_CXX_COMPILER_ID}=1)
add_compile_definitions(eLibrarySystem_${CMAKE_SYSTEM_NAME}=1)

if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    add_compile_options(/wd4514 /wd4577 /wd4668 /wd4710 /wd4711 /wd4820 /wd4996 /wd5045 /wd5267)
endif()

add_library(eLibrary eLibrary/Core/Object.cpp eLibrary/Core/String.cpp)

if(ELIBRARY_UNIT_TEST)
    enable_testing()
    add_subdirectory(eLibraryTest)
endif()
