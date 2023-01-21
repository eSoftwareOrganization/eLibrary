project(eLibraryTest)

include_directories(../eLibrary)

if(WIN32)
link_libraries(ntdll.dll shlwapi.lib)
endif()

add_executable(eLibraryTest eLibraryTest.cpp)

target_link_libraries(eLibraryTest eLibrary)
