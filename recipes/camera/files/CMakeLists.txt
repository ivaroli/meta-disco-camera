cmake_minimum_required(VERSION 3.7)

# flags for cross complination
set(CMAKE_LIBRARY_ARCHITECTURE aarch64-linux-gnu)
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_C_COMPILER /usr/bin/aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER /usr/bin/aarch64-linux-gnu-g++)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_FIND_ROOT_PATH ${STAGING_DIR_HOST} ${STAGING_DIR_NATIVE} ${CROSS_DIR})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_SKIP_BUILD_RPATH  TRUE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)

# for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

project(Disco2CameraControl VERSION 1.0 LANGUAGES CXX)

EXECUTE_PROCESS( COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE ARCHITECTURE )
message( STATUS "Architecture: ${ARCHITECTURE}" )

set(Vmb_DIR ${CMAKE_SOURCE_DIR}/lib/VimbaX_2023-1/api/lib/cmake/vmb)

find_package(Vmb REQUIRED COMPONENTS CPP)
find_package(OpenCV REQUIRED)

add_subdirectory(src/helpers)
add_subdirectory(src/providers)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/libcsp)

add_executable(Disco2CameraControl src/main.cpp)

target_link_libraries(Disco2CameraControl PRIVATE Vmb::CPP ${OpenCV_LIBS} libproviders libhelpers csp)
target_include_directories(Disco2CameraControl PRIVATE ${CMAKE_SOURCE_DIR}/src/include/helpers ${CMAKE_SOURCE_DIR}/src/include/providers ${CMAKE_SOURCE_DIR}/src/include)