project(eLibraryTest)

include_directories(../eLibrary)

if(WIN32)
link_libraries(ntdll.dll shlwapi.lib ws2_32.dll)
endif()

find_package(doctest CONFIG REQUIRED)

include_directories(${doctest_INCLUDE_DIRS})

add_executable(eLibraryTest eLibraryTest.cpp)

target_link_libraries(eLibraryTest eLibrary)
target_link_libraries(eLibraryTest doctest::doctest)
