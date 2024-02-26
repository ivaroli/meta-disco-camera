cmake_minimum_required(VERSION 3.0)

project(VmbCPP)

set(VMB_CPP_TARGET_NAME "VmbCPP" CACHE STRING "The name to use for the VmbCPP version compiled from source")

# make sure the configuration scripts for Vmb can be found
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../..")

find_package(Vmb REQUIRED COMPONENTS CPP-sources)

vmb_create_cpp_target(TARGET_NAME ${VMB_CPP_TARGET_NAME})
