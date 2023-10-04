cmake_minimum_required(VERSION 3.17)

cmake_policy(SET CMP0003 NEW)
cmake_policy(SET CMP0005 NEW)

option(ELIBRARY_MODULE_IO "eLibrary::IO" ON)
option(ELIBRARY_MODULE_MULTIMEDIA "eLibrary::Multimedia" OFF)
option(ELIBRARY_MODULE_NETWORK "eLibrary::Network" ON)
option(ELIBRARY_PACKAGE OFF)
option(ELIBRARY_UNIT_TEST OFF)

if(ELIBRARY_MODULE_IO)
    add_compile_definitions(eLibraryFeature_IO=1)
endif()

if(ELIBRARY_MODULE_MULTIMEDIA)
    list(APPEND VCPKG_MANIFEST_FEATURES "module-multimedia")
    add_compile_definitions(eLibraryFeature_Multimedia=1)
endif()

if(ELIBRARY_MODULE_NETWORK)
    add_compile_definitions(eLibraryFeature_Network=1)
endif()

project(eLibrary DESCRIPTION "An all-in-one cpp library" VERSION 2023.17.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

include_directories(eLibrary)

add_compile_options(-Wall)

add_compile_definitions(eLibraryCompiler_${CMAKE_CXX_COMPILER_ID}=1)
add_compile_definitions(eLibrarySystem_${CMAKE_SYSTEM_NAME}=1)

if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    add_compile_options(/wd4514 /wd4577 /wd4623 /wd4668 /wd4710 /wd4711 /wd4820 /wd4996 /wd5045 /wd5267)
endif()

add_library(eLibrary eLibrary/Core/Object.cpp eLibrary/Core/String.cpp)

if(ELIBRARY_PACKAGE)
    install(DIRECTORY eLibrary eLibraryDocumentation eLibraryTest DESTINATION .)
    install(FILES CMakeLists.txt LICENSE ReadMe.md DESTINATION .)
    set(CPACK_BUILD_SOURCE_DIRS "${CMAKE_HOME_DIRECTORY}/eLibrary")
    set(CPACK_GENERATOR "TGZ")
    set(CPACK_PACKAGE_VENDOR "eSoftware")
    include(CPack)
endif()

enable_testing()
if(ELIBRARY_UNIT_TEST)
    add_subdirectory(eLibraryTest)
endif()
