cmake_minimum_required(VERSION 3.0)

project(ChunkAccess LANGUAGES CXX)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/vmb_cmake_prefix_paths.cmake")
    # read hardcoded package location information, if the example is still located in the original install location
    include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/vmb_cmake_prefix_paths.cmake)
endif()

find_package(Vmb REQUIRED COMPONENTS CPP)

if(NOT WIN32)
    list(APPEND CMAKE_EXE_LINKER_FLAGS "-Wl,-rpath,'\$ORIGIN'")
endif()

add_executable(ChunkAccess_VmbCPP
    main.cpp
    AcquisitionHelperChunk.cpp
    AcquisitionHelperChunk.h
)

target_link_libraries(ChunkAccess_VmbCPP PRIVATE Vmb::CPP )
set_target_properties(ChunkAccess_VmbCPP PROPERTIES
    CXX_STANDARD 11
    VS_DEBUGGER_ENVIRONMENT "PATH=${VMB_BINARY_DIRS};$ENV{PATH}"
)
