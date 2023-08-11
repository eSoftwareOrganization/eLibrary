project(eLibraryTest)

include_directories(../eLibrary)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    link_libraries(ntdll.dll shlwapi.lib ws2_32.dll)
endif()

find_package(doctest CONFIG REQUIRED)
find_package(nanobench CONFIG REQUIRED)

include_directories(${doctest_INCLUDE_DIRS})

add_executable(eLibraryTest eLibraryTest.cpp)

target_link_libraries(eLibraryTest eLibrary doctest::doctest)
if(BUILD_MODULE_IO)
    find_package(LibArchive REQUIRED)
    target_link_libraries(eLibraryTest LibArchive::LibArchive)
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
        find_package(dlfcn-win32 CONFIG REQUIRED)
        target_link_libraries(eLibraryTest dlfcn-win32::dl)
    endif()
endif()
if(BUILD_MODULE_MULTIMEDIA)
    find_package(FFMPEG REQUIRED)
    find_package(OpenAL CONFIG REQUIRED)
    target_link_libraries(eLibraryTest ${FFMPEG_LIBRARIES} OpenAL::OpenAL)
endif()
if(BUILD_MODULE_NETWORK)
    find_package(OpenSSL REQUIRED)
    target_link_libraries(eLibraryTest OpenSSL::SSL OpenSSL::Crypto)
endif()
