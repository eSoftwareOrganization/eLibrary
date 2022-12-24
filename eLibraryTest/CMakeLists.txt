project(eLibraryTest)

include_directories(../eLibrary)

#add_compile_definitions(eLibraryDoctest)

add_compile_options(-mfma)

if(WIN32)
link_libraries(shlwapi.lib)
endif()

add_executable(eLibraryTest eLibraryTest.cpp)

target_link_libraries(eLibraryTest eLibrary)
