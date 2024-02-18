project(eLibraryTest)

include_directories(${CMAKE_HOME_DIRECTORY}/eLibrary)

if(ELIBRARY_MODULE_NETWORK AND ${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    link_libraries(ws2_32)
endif()

add_executable(eLibraryTest eLibraryTest.cpp)

target_link_libraries(eLibraryTest eLibrary)

if(ELIBRARY_MODULE_EXTENSION_GEOIP)
    find_package(PkgConfig REQUIRED)
    pkg_search_module(libmaxminddb REQUIRED libmaxminddb)
    include_directories(${libmaxminddb_INCLUDE_DIRS})
    target_link_libraries(eLibraryTest ${libmaxminddb_LIBRARY})
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
