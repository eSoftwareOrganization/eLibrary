cmake_minimum_required(VERSION 3.0)

project(eLibrary)

include_directories(eLibrary)

link_libraries(ntdll.dll)

if (WIN32)
add_compile_definitions(eLibraryCore_WindowsNT)
endif()
add_compile_definitions(eLibraryIO_FastBufferedStream)

add_compile_options(-Wall -Ofast -mfma)

set(CMAKE_CXX_STANDARD 20)

add_library(eLibrary eLibrary/Core/String.cpp)

add_subdirectory(eLibraryTest)
