cmake_minimum_required(VERSION 3.10)

project(eLibrary)

include_directories(eLibrary)

if (WIN32)
add_compile_definitions(eLibraryCore_WindowsNT)
endif()

add_compile_options(-march=native -Wall)

set(CMAKE_CXX_STANDARD 20)

add_library(eLibrary eLibrary/Core/Object.cpp eLibrary/Core/String.cpp)

add_subdirectory(eLibraryTest)
