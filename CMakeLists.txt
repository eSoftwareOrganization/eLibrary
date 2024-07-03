cmake_minimum_required(VERSION 3.17)

cmake_policy(SET CMP0003 NEW)
cmake_policy(SET CMP0005 NEW)

option(ELIBRARY_COMPACT_STL "Currently no effects" ON)
option(ELIBRARY_LINKAGE_DYNAMIC "ON for dynamical linkage, OFF for statical linkage(Default OFF)" OFF)
option(ELIBRARY_MODULE_IO "eLibrary::IO(Default ON)" ON)
option(ELIBRARY_MODULE_MULTIMEDIA "eLibrary::Multimedia(Default OFF)" OFF)
option(ELIBRARY_MODULE_NETWORK "eLibrary::Network(Default ON)" ON)
option(ELIBRARY_PACKAGE OFF)
option(ELIBRARY_UNIT_TEST OFF)

if(ELIBRARY_COMPACT_STL)
    add_compile_definitions(eLibraryCompact_STL=1)
endif()

if(ELIBRARY_MODULE_IO)
    add_compile_definitions(eLibraryFeature_IO=1)
endif()

if(ELIBRARY_MODULE_MULTIMEDIA)
    list(APPEND VCPKG_MANIFEST_FEATURES "module-multimedia")
    add_compile_definitions(eLibraryFeature_Multimedia=1)
endif()

if(ELIBRARY_MODULE_NETWORK)
    add_compile_definitions(eLibraryFeature_Network=1)
endif()

project(eLibrary DESCRIPTION "An all-in-one cpp library" VERSION 0.21.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

include_directories(eLibrary)

add_compile_options(-Wall)

add_compile_definitions(eLibraryArchitecture_${CMAKE_SYSTEM_PROCESSOR}=1)
add_compile_definitions(eLibraryCompiler_${CMAKE_CXX_COMPILER_ID}=1)
add_compile_definitions(eLibrarySystem_${CMAKE_SYSTEM_NAME}=1)

if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    add_compile_options(/wd4514 /wd4577 /wd4623 /wd4668 /wd4710 /wd4711 /wd4820 /wd4996 /wd5045 /wd5267)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

file(GLOB_RECURSE FileList "eLibrary/*")
if(ELIBRARY_LINKAGE_DYNAMIC)
    add_compile_definitions(eLibraryLinkage_Dynamic=1)
    add_library(eLibrary SHARED ${FileList})
else()
    add_compile_definitions(eLibraryLinkage_Static=1)
    add_library(eLibrary STATIC ${FileList})
endif()

set_target_properties(eLibrary PROPERTIES PUBLIC_HEADER ${CMAKE_HOME_DIRECTORY}/eLibrary/eLibrary.hpp)

include(CMakePackageConfigHelpers)
configure_package_config_file(
        eLibraryConfig.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/eLibraryConfig.cmake"
        INSTALL_DESTINATION lib/cmake/eLibrary
)
write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/eLibraryConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
)
install(DIRECTORY eLibrary DESTINATION eLibrary)
install(
        TARGETS eLibrary
        EXPORT eLibraryTargets
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        PUBLIC_HEADER DESTINATION eLibrary
)
install(EXPORT eLibraryTargets DESTINATION lib/cmake/eLibrary NAMESPACE eLibrary::)
install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/eLibraryConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/eLibraryConfigVersion.cmake
        DESTINATION lib/cmake/eLibrary
)

if(ELIBRARY_PACKAGE)
    include(InstallRequiredSystemLibraries)
    set(CPACK_GENERATOR "ZIP")
    set(CPACK_PACKAGE_VENDOR "eSoftware")
    set(CPACK_PROJECT_NAME ${PROJECT_NAME})
    set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_HOME_DIRECTORY}/LICENSE")
    include(CPack)
endif()

if(ELIBRARY_UNIT_TEST)
    include(CTest)
    enable_testing()
    add_subdirectory(eLibraryTest)
endif()
