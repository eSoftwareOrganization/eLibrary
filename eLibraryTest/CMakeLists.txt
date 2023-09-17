project(eLibraryTest)

include_directories(${CMAKE_HOME_DIRECTORY}/eLibrary)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    link_libraries(ntdll.dll shlwapi.lib ws2_32.dll)
endif()

add_executable(eLibraryTest eLibraryTest.cpp)

target_link_libraries(eLibraryTest eLibrary)
if(ELIBRARY_MODULE_IO)
    if(ELIBRARY_FEATURE_ARCHIVE)
        find_package(LibArchive REQUIRED)
        include_directories(${LibArchive_INCLUDE_DIRS})
        target_link_libraries(eLibraryTest LibArchive::LibArchive)
    endif()
endif()
if(ELIBRARY_MODULE_MULTIMEDIA)
    find_package(FFMPEG REQUIRED)
    include_directories(${FFMPEG_INCLUDE_DIRS})
    find_package(OpenAL CONFIG REQUIRED)
    include_directories(${OpenAL_INCLUDE_DIRS})
    target_link_libraries(eLibraryTest ${FFMPEG_LIBRARIES} OpenAL::OpenAL)
endif()

add_test(
    NAME eLibraryTest
    COMMAND eLibraryTest
)
