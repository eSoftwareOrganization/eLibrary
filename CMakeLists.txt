cmake_minimum_required(VERSION 3.0)

project(eLibrary)

include_directories(eLibrary)

if (WIN32)
add_compile_definitions(eLibraryCore_WindowsNT)
endif()
add_compile_definitions(eLibraryIO_FastBufferedStream)

add_compile_options(-march=native -Ofast -Wall)

set(CMAKE_CXX_STANDARD 20)

add_library(eLibrary eLibrary/Core/String.cpp)

add_subdirectory(eLibraryTest)
