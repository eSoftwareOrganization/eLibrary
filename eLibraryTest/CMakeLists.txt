project(eLibraryTest)

include_directories(../eLibrary)

if(WIN32)
link_libraries(ntdll.dll shlwapi.lib ws2_32.dll)
endif()

find_package(doctest CONFIG REQUIRED)
find_package(FFMPEG REQUIRED)
find_package(nanobench CONFIG REQUIRED)
find_package(OpenAL CONFIG REQUIRED)

include_directories(${doctest_INCLUDE_DIRS})

add_executable(eLibraryTest eLibraryTest.cpp)

target_include_directories(eLibraryTest PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(eLibraryTest PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(eLibraryTest eLibrary doctest::doctest ${FFMPEG_LIBRARIES} nanobench::nanobench OpenAL::OpenAL)
