project(eLibraryTest)

include_directories(../eLibrary)

link_libraries(ws2_32.lib shlwapi.lib)

add_executable(eLibraryTest eLibraryTest.cpp)

target_link_libraries(eLibraryTest eLibrary)
