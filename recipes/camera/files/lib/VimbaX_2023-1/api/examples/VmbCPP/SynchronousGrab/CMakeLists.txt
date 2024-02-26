cmake_minimum_required(VERSION 3.0)

project(SynchronousGrab LANGUAGES CXX)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/vmb_cmake_prefix_paths.cmake")
    # read hardcoded package location information, if the example is still located in the original install location
    include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/vmb_cmake_prefix_paths.cmake)
endif()

find_package(Vmb REQUIRED COMPONENTS CPP)

if(NOT WIN32)
    list(APPEND CMAKE_EXE_LINKER_FLAGS "-Wl,-rpath,'\$ORIGIN'")
endif()

add_executable(SynchronousGrab_VmbCPP
    main.cpp
    SynchronousGrab.cpp
    SynchronousGrab.h   
)

target_link_libraries(SynchronousGrab_VmbCPP PRIVATE Vmb::CPP )
set_target_properties(SynchronousGrab_VmbCPP PROPERTIES
    CXX_STANDARD 11
    VS_DEBUGGER_ENVIRONMENT "PATH=${VMB_BINARY_DIRS};$ENV{PATH}"
)