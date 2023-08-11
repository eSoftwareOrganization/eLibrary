cmake_minimum_required(VERSION 3.17)

option(BUILD_MODULE_IO "Build eLibrary::IO" ON)
if(BUILD_MODULE_IO)
    list(APPEND VCPKG_MANIFEST_FEATURES "module-io")
    add_compile_definitions(eLibraryFeature_IO=1)
endif()

option(BUILD_MODULE_MULTIMEDIA "Build eLibrary::Multimedia" ON)
if(BUILD_MODULE_MULTIMEDIA)
    list(APPEND VCPKG_MANIFEST_FEATURES "module-multimedia")
    add_compile_definitions(eLibraryFeature_Multimedia=1)
endif()

option(BUILD_MODULE_NETWORK "Build eLibrary::Network" ON)
if(BUILD_MODULE_NETWORK)
    list(APPEND VCPKG_MANIFEST_FEATURES "module-network")
    add_compile_definitions(eLibraryFeature_Network=1)
endif()

option(BUILD_APPLICATION_TEST "Build eLibrary::eLibraryTest" ON)
if(BUILD_APPLICATION_TEST)
    list(APPEND VCPKG_MANIFEST_FEATURES "application-test")
endif()

project(eLibrary)

set(CMAKE_CXX_STANDARD 23)

include_directories(eLibrary)

add_compile_options(-Wall)
if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    add_compile_options(-Wextra)
endif()

add_compile_definitions(eLibraryCompiler_${CMAKE_CXX_COMPILER_ID}=1)
add_compile_definitions(eLibrarySystem_${CMAKE_SYSTEM_NAME}=1)

add_library(eLibrary eLibrary/Core/Object.cpp eLibrary/Core/String.cpp)

if(BUILD_APPLICATION_TEST)
    add_subdirectory(eLibraryTest)
endif()
