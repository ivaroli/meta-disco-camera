# project template for VmbCPP

if (CMAKE_VERSION VERSION_LESS 3.5)
    find_package(CMakeParseArguments MODULE REQUIRED)
endif()

# usage: vmb_argument(<var name without PARAMS_ prefix> [DEFAULT <default>] [POSSIBILITIES <possibility>...]
function(vmb_argument VAR)
    cmake_parse_arguments(
        PARAMS
        ""              # options
        "DEFAULT"       # single value params
        "POSSIBILITIES" # multi value params
        ${ARGN}
    )
    if (NOT DEFINED "PARAMS_${VAR}")
        if (DEFINED PARAMS_DEFAULT)
            set("PARAMS_${VAR}" ${PARAMS_DEFAULT} PARENT_SCOPE)
        endif()
    elseif(DEFINED PARAMS_POSSIBILITIES)
        list(FIND PARAMS_POSSIBILITIES ${PARAMS_${VAR}} FOUND_INDEX)
        if (FOUND_INDEX EQUAL -1)
            message(FATAL_ERROR "invalid value for ${VAR}: ${PARAMS_${VAR}}\nexpected one of ${PARAMS_POSSIBILITIES}")
        endif()
    endif()
endfunction()

function(vmb_source_group VAR DIR GROUP_NAME)
    set(SRCS)
    foreach(_SOURCE IN ITEMS ${ARGN})
        list(APPEND SRCS "${DIR}/${_SOURCE}")
    endforeach()
    source_group(${GROUP_NAME} FILES ${SRCS})
    set(${VAR} ${${VAR}} ${SRCS} PARENT_SCOPE)
endfunction()

set(VMB_CPP_TEMPLATE_SCRIPT_DIR ${CMAKE_CURRENT_LIST_DIR} CACHE INTERNAL "Path to VmbCppTemplate.cmake")

# parameters:
#  * required:
#    - TARGET_NAME <name>
#  * optional:
#   - TYPE [STATIC|SHARED]                                      the library type; defaults to SHARED 
#   - USER_INCLUDE_DIRS [<dir>...]                              directories to append to the include dirs
#   - GENERATED_INCLUDES_DIR <dir>                              directory to place the generated header in;
#                                                               defaults to "${CMAKE_CURRENT_BINARY_DIR}/VmbCppGenIncludes"
#
#   - FILE_LOGGER_HEADER <header name to use in config.h>       the parameter following this one lists the file name of the custom file logger define header;
#                                                               defaults to VmbCPP/UserLoggerDefines.h
#
#   - SHARED_POINTER_HEADER <header name to use in config.h>    the parameter following this one lists the file name of the custom shared pointer header;
#                                                               defaults to VmbCPP/UserSharedPointerDefines.h
#
#   - ADDITIONAL_SOURCES [<src>...]                             sources to add to the sources of the target
#   - NO_RC                                                     exclude rc file on windows (automatically done for non-shared
#
#   - DOXYGEN_TARGET <target-name>                              Specify the name of the target for generating the doxygen docu, if doxygen is found
#
#   - VMBC_INCLUDE_ROOT <directory-path>                        Specify the directory to get VmbC public headers from; defaults to the include dir VmbCPP
#
# Generates a target for the VmbCPP library of the given name.
#
# If ADDITIONAL_SOURCES is present, the sources are added to the target's sources
#
function(vmb_create_cpp_target PARAM_1 PARAM_2)
    set(OPTION_PARAMS NO_RC)
    set(SINGLE_VALUE_PARAMS
        TARGET_NAME TYPE
        GENERATED_INCLUDES_DIR
        FILE_LOGGER_HEADER
        SHARED_POINTER_HEADER
        DOXYGEN_TARGET
        VMBC_INCLUDE_ROOT
    )
    set(MULTI_VALUE_PARAMS
        USER_INCLUDE_DIRS
        ADDITIONAL_SOURCES
    )
    
    cmake_parse_arguments(
        PARAMS
        "${OPTION_PARAMS}"
        "${SINGLE_VALUE_PARAMS}"
        "${MULTI_VALUE_PARAMS}"
        ${ARGV}
    )
    
    if(NOT PARAMS_TARGET_NAME)
        message(FATAL_ERROR "Required parameter TARGET_NAME not specified")
    endif()
    
    vmb_argument(TYPE DEFAULT SHARED POSSIBILITIES SHARED STATIC)
    vmb_argument(GENERATED_INCLUDES_DIR DEFAULT "${CMAKE_CURRENT_BINARY_DIR}/VmbCppGenIncludes")

    if(EXISTS "${VMB_CPP_TEMPLATE_SCRIPT_DIR}/../../../../bin/VmbC.dll" OR EXISTS "${VMB_CPP_TEMPLATE_SCRIPT_DIR}/../../../../lib/libVmbC.so")
        # installed version
        get_filename_component(VMBCPP_SOURCE_DIR "${VMB_CPP_TEMPLATE_SCRIPT_DIR}/../../../../source/VmbCPP" ABSOLUTE)
        get_filename_component(VMBCPP_INCLUDE_DIR "${VMB_CPP_TEMPLATE_SCRIPT_DIR}/../../../../include" ABSOLUTE)
    else()
        # build by vendor
        get_filename_component(VMBCPP_SOURCE_DIR "${VMB_CPP_TEMPLATE_SCRIPT_DIR}/../Source/VmbCPP_internal" ABSOLUTE)
        get_filename_component(VMBCPP_INCLUDE_DIR "${VMB_CPP_TEMPLATE_SCRIPT_DIR}/../Include" ABSOLUTE)
    endif()

    set(SOURCES)
    set(HEADERS)
    set(PUBLIC_HEADERS)

    vmb_source_group(SOURCES ${VMBCPP_SOURCE_DIR} Base
        BasicLockable.cpp
        Clock.cpp
        Condition.cpp
        ConditionHelper.cpp
        FileLogger.cpp
        Mutex.cpp
        MutexGuard.cpp
        Semaphore.cpp
    )

    vmb_source_group(PUBLIC_HEADERS "${VMBCPP_INCLUDE_DIR}/VmbCPP" Base
        BasicLockable.h
        FileLogger.h
        LoggerDefines.h
        Mutex.h
        SharedPointer.h
        SharedPointerDefines.h
        SharedPointer_impl.h
        VmbCPP.h
        VmbCPPCommon.h
    )

    vmb_source_group(HEADERS ${VMBCPP_SOURCE_DIR} Base
        Clock.h
        Condition.h
        ConditionHelper.h
        Helper.h
        MutexGuard.h
        Semaphore.h
    )

    vmb_source_group(SOURCES ${VMBCPP_SOURCE_DIR} Features
        BaseFeature.cpp
        BoolFeature.cpp
        CommandFeature.cpp
        EnumEntry.cpp
        EnumFeature.cpp
        Feature.cpp
        FloatFeature.cpp
        IntFeature.cpp
        RawFeature.cpp
        StringFeature.cpp
    )

    vmb_source_group(HEADERS ${VMBCPP_SOURCE_DIR} Features
        BaseFeature.h
        BoolFeature.h
        CommandFeature.h
        EnumFeature.h
        FloatFeature.h
        IntFeature.h
        RawFeature.h
        StringFeature.h
    )

    vmb_source_group(PUBLIC_HEADERS "${VMBCPP_INCLUDE_DIR}/VmbCPP" Features
        Feature.h
        EnumEntry.h
    )

    vmb_source_group(PUBLIC_HEADERS "${VMBCPP_INCLUDE_DIR}/VmbCPP" Features\\Inline
        EnumEntry.hpp
        Feature.hpp
    )
    
    vmb_source_group(PUBLIC_HEADERS "${VMBCPP_INCLUDE_DIR}/VmbCPP" Utils\\Inline
        CopyHelper.hpp
        UniquePointer.hpp
    )

    vmb_source_group(SOURCES ${VMBCPP_SOURCE_DIR} Modules
        TransportLayer.cpp
        Camera.cpp
        DefaultCameraFactory.cpp
        FeatureContainer.cpp
        Interface.cpp
        VmbSystem.cpp
        LocalDevice.cpp
        Stream.cpp
        PersistableFeatureContainer.cpp
    )
    
    vmb_source_group(SOURCES ${VMBCPP_SOURCE_DIR} Utils\\Inline
        CopyUtils.hpp
    )

    vmb_source_group(PUBLIC_HEADERS "${VMBCPP_INCLUDE_DIR}/VmbCPP" Modules
        TransportLayer.h
        Camera.h
        FeatureContainer.h
        Interface.h
        VmbSystem.h
        LocalDevice.h
        Stream.h
        PersistableFeatureContainer.h
    )

    vmb_source_group(PUBLIC_HEADERS "${VMBCPP_INCLUDE_DIR}/VmbCPP" Modules\\Inline
        TransportLayer.hpp
        Camera.hpp
        FeatureContainer.hpp
        Interface.hpp
        VmbSystem.hpp
    )

    vmb_source_group(PUBLIC_HEADERS "${VMBCPP_INCLUDE_DIR}/VmbCPP" Utils\\Inline
        StringLike.hpp
    )

    vmb_source_group(HEADERS ${VMBCPP_SOURCE_DIR} Modules
        DefaultCameraFactory.h
    )


    vmb_source_group(PUBLIC_HEADERS "${VMBCPP_INCLUDE_DIR}/VmbCPP" Interfaces
        ICameraFactory.h
        ICameraListObserver.h
        IFeatureObserver.h
        IFrameObserver.h
        IInterfaceListObserver.h
        ICapturingModule.h
    )

    vmb_source_group(SOURCES ${VMBCPP_SOURCE_DIR} Interfaces
        IFrameObserver.cpp
    )


    vmb_source_group(SOURCES ${VMBCPP_SOURCE_DIR} Frame
        Frame.cpp
        FrameHandler.cpp
    )

    vmb_source_group(PUBLIC_HEADERS "${VMBCPP_INCLUDE_DIR}/VmbCPP" Frame
        Frame.h
    )
    
    vmb_source_group(HEADERS ${VMBCPP_SOURCE_DIR} "Frame"
        FrameHandler.h
        FrameImpl.h
    )

    vmb_source_group(PUBLIC_HEADERS "${VMBCPP_INCLUDE_DIR}/VmbCPP" Frame\\Inline
        Frame.hpp
    )

    vmb_source_group(PUBLIC_HEADERS "${VMBCPP_INCLUDE_DIR}/VmbCPP" "User"
        UserLoggerDefines.h
        UserSharedPointerDefines.h
    )

    # Note: the config file contains no header guard on purpose
    set(_CONTENT "// VmbCPP configuration file")

    function(vmb_cpp_add_config_header VAR DEFINITION DEFAULT HEADER_GUARD DEFAULT_INCLUDE)
        if (DEFINED "PARAMS_${VAR}")
            set(_CONTENT "${_CONTENT}

#define ${DEFINITION}

#ifdef ${HEADER_GUARD}
//   include only from DEFAULT_INCLUDE
#    include \"${PARAMS_${VAR}}\"
#endif
" PARENT_SCOPE)
        endif()
    endfunction()

    vmb_cpp_add_config_header(FILE_LOGGER_HEADER USER_LOGGER VmbCPP/UserLoggerDefines.h VMBCPP_LOGGERDEFINES_H LoggerDefines.h)
    vmb_cpp_add_config_header(SHARED_POINTER_HEADER USER_SHARED_POINTER VmbCPP/UserSharedPointerDefines.h VMBCPP_SHAREDPOINTERDEFINES_H SharedPointerDefines.h)
    
    file(MAKE_DIRECTORY "${PARAMS_GENERATED_INCLUDES_DIR}/VmbCPPConfig")
    set(_CONFIG_FILE "${PARAMS_GENERATED_INCLUDES_DIR}/VmbCPPConfig/config.h")
    set(_OLD_CONFIG_CONTENT "")
    get_filename_component(_CONFIG_FILE_ABSOLUTE ${_CONFIG_FILE} ABSOLUTE)
    if(EXISTS "${_CONFIG_FILE_ABSOLUTE}")
        file(READ ${_CONFIG_FILE_ABSOLUTE} _OLD_CONFIG_CONTENT)
    endif()
    if(NOT _OLD_CONFIG_CONTENT STREQUAL _CONTENT)
        file(WRITE ${_CONFIG_FILE} ${_CONTENT})
    endif()
    
    list(APPEND HEADERS ${_CONFIG_FILE})

    source_group("Generated" FILES ${_CONFIG_FILE})

    if (PARAMS_TYPE STREQUAL "SHARED" AND NOT PARAMS_NO_RC)
        if (WIN32)
            vmb_source_group(SOURCES ${VMBCPP_SOURCE_DIR} Resources
                    resource.h
                    VmbCPP.rc
                    VmbCPP.rc2
            )
        endif()
    endif()

    find_package(Vmb REQUIRED COMPONENTS C)
    set(VMB_MAJOR_VERSION ${Vmb_VERSION_MAJOR})
    set(VMB_MINOR_VERSION ${Vmb_VERSION_MINOR})
    set(VMB_PATCH_VERSION ${Vmb_VERSION_PATCH})

    set(VMBCPP_VERSION_HEADER "${PARAMS_GENERATED_INCLUDES_DIR}/Version.h")
    configure_file("${VMBCPP_SOURCE_DIR}/Version.h.in" ${VMBCPP_VERSION_HEADER})

    list(APPEND HEADERS ${VMBCPP_VERSION_HEADER})

    source_group("Generated" FILES
        ${VMBCPP_VERSION_HEADER}
    )

    add_library(${PARAMS_TARGET_NAME} ${PARAMS_TYPE}
        ${HEADERS}
        ${PUBLIC_HEADERS}
        ${SOURCES}
        ${PARAMS_ADDITIONAL_SOURCES}
        ${VMBCPP_VERSION_HEADER}
        ${VMB_CPP_TWEAK_VERSION_HEADER}
    )
    
    if(${PARAMS_TYPE} STREQUAL "SHARED")
        target_compile_definitions(${PARAMS_TARGET_NAME} PRIVATE VMBCPP_CPP_EXPORTS)
    else()
        target_compile_definitions(${PARAMS_TARGET_NAME} PRIVATE VMBCPP_CPP_LIB)
    endif()

    target_link_libraries(${PARAMS_TARGET_NAME} PUBLIC Vmb::C)
    target_include_directories(${PARAMS_TARGET_NAME} PUBLIC
        $<BUILD_INTERFACE:${VMBCPP_INCLUDE_DIR}>
    )

    # make sure the generated include is preferred to the one installed with the sdk
    target_include_directories(${PARAMS_TARGET_NAME} BEFORE PUBLIC
        $<BUILD_INTERFACE:${PARAMS_GENERATED_INCLUDES_DIR}>
    )

    foreach(_DIR IN LISTS PARAMS_USER_INCLUDE_DIRS)
        get_filename_component(_DIR_ABS ${_DIR} ABSOLUTE)
        target_include_directories(${PARAMS_TARGET_NAME} PUBLIC $<BUILD_INTERFACE:${_DIR_ABS}>)
    endforeach()
    
    set_target_properties(${PARAMS_TARGET_NAME} PROPERTIES
        PUBLIC_HEADER "${PUBLIC_HEADERS}"
    )

    if (CMAKE_VERSION VERSION_LESS 3.8)
        # compile features for standard added in v 3.8
        # -> set the version for the created target only
        set_target_properties(${PARAMS_TARGET_NAME} PROPERTIES
            CXX_STANDARD 11
        )
    else()
        # target and linking target is required to use at least C++11
        target_compile_features(${PARAMS_TARGET_NAME} PUBLIC cxx_std_11)
    endif()

    if (PARAMS_DOXYGEN_TARGET)
        if (CMAKE_VERSION VERSION_LESS 3.9)
            message(WARNING "At least version 3.9 CMake required for doxygen documentation")
        else()
            find_package(Doxygen)
            if (DOXYGEN_FOUND)
                if (NOT CMAKE_FOLDER)
                    set(CMAKE_FOLDER Documentation)
                endif()

                if (NOT PARAMS_VMBC_INCLUDE_ROOT)
                    set(PARAMS_VMBC_INCLUDE_ROOT ${VMBCPP_INCLUDE_DIR})
                endif()

                set(VMBC_HEADERS
                    "${PARAMS_VMBC_INCLUDE_ROOT}/VmbC/VmbC.h"
                    "${PARAMS_VMBC_INCLUDE_ROOT}/VmbC/VmbCommonTypes.h"
                    "${PARAMS_VMBC_INCLUDE_ROOT}/VmbC/VmbConstants.h"
                    "${PARAMS_VMBC_INCLUDE_ROOT}/VmbC/VmbCTypeDefinitions.h"
                )
                
                set(DOXYGEN_GENERATE_XML YES)

                doxygen_add_docs(${PARAMS_DOXYGEN_TARGET} ${HEADERS} ${PUBLIC_HEADERS} ${VMBC_HEADERS})
            endif()
        endif()
    endif()
endfunction()
