cmake_minimum_required(VERSION 3.10)

project(eLibrary)

include_directories(eLibrary)

add_compile_options(-Wall)

set(CMAKE_CXX_STANDARD 20)

add_library(eLibrary eLibrary/Core/Object.cpp eLibrary/Core/String.cpp)

add_subdirectory(eLibraryTest)
