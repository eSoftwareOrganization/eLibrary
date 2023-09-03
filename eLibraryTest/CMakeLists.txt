project(eLibraryTest)

include_directories(../eLibrary)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    link_libraries(ntdll.dll shlwapi.lib ws2_32.dll)
endif()

find_package(doctest CONFIG REQUIRED)
find_package(nanobench CONFIG REQUIRED)

include_directories(${doctest_INCLUDE_DIRS})

add_executable(eLibraryTest eLibraryTest.cpp)

target_link_libraries(eLibraryTest eLibrary)
if(ELIBRARY_MODULE_IO)
    if(ELIBRARY_FEATURE_ARCHIVE)
        find_package(LibArchive REQUIRED)
        target_link_libraries(eLibraryTest LibArchive::LibArchive)
    endif()
endif()
if(ELIBRARY_MODULE_MULTIMEDIA)
    find_package(FFMPEG REQUIRED)
    find_package(OpenAL CONFIG REQUIRED)
    target_link_libraries(eLibraryTest ${FFMPEG_LIBRARIES} OpenAL::OpenAL)
endif()
if(ELIBRARY_MODULE_NETWORK)
    find_package(OpenSSL REQUIRED)
    target_link_libraries(eLibraryTest OpenSSL::SSL OpenSSL::Crypto)
endif()

add_test(
    NAME eLibraryTest
    COMMAND eLibraryTest
)
