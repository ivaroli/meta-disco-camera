
####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was vmb-config.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

set(${CMAKE_FIND_PACKAGE_NAME}_FOUND 0)

set(_VMB_FIND_PACKAGE_IMPL_FILES)

function(_vmb_find_package_impl)
    if (${CMAKE_FIND_PACKAGE_NAME}_FIND_QUIETLY)
        set(_FIND_COMPONENTS_QUIET QUIET)
        macro(_vmb_find_message MESSAGE_TYPE)
            if (MESSAGE_TYPE STREQUAL "FATAL_ERROR")
                return()
            endif()
        endmacro()
    else()
        macro(_vmb_find_message)
            message(${ARGN})
        endmacro()
    endif()

    # turn a input string into the name used for files/internal variables
    function(_vmb_to_component_string VAR COMPONENT)
        string(REPLACE "-" "_" RESULT ${COMPONENT})
        string(TOLOWER ${RESULT} RESULT)
        set(${VAR} ${RESULT} PARENT_SCOPE)
    endfunction()

    if (NOT WIN32 AND NOT UNIX)
        _vmb_find_message(FATAL_ERROR "Not yet implemented for the target system ${CMAKE_SYSTEM_NAME}")
    endif()

    # default components: only VmbC
    set(VMB_FIND_FILES "${PACKAGE_PREFIX_DIR}/lib/cmake/vmb/configs/vmb_c.cmake")

    # prerequesites of each component (VMB_<component>_DEPENDENCIES = include file name)
    set(VMB_c_DEPENDENCIES)
    set(VMB_cpp_DEPENDENCIES vmb_c)
    set(VMB_cpp_sources_DEPENDENCIES vmb_c)
    set(VMB_imagetransform_DEPENDENCIES)

    if (CMAKE_VERSION VERSION_LESS 3.0)
        _vmb_find_message(FATAL_ERROR "CMake versions < 3.0 are not supported")
    endif()

    if(${CMAKE_FIND_PACKAGE_NAME}_FIND_COMPONENTS)
        set(VMB_FIND_FILES)
        foreach(_COMP IN LISTS ${CMAKE_FIND_PACKAGE_NAME}_FIND_COMPONENTS)
            _vmb_to_component_string(INCLUDE_FILE_NAME ${_COMP})
            if(EXISTS "${PACKAGE_PREFIX_DIR}/lib/cmake/vmb/configs/vmb_${INCLUDE_FILE_NAME}.cmake")
                set(DEPENDENCIES)
                foreach(DEP IN LISTS VMB_${INCLUDE_FILE_NAME}_DEPENDENCIES)
                    list(APPEND DEPENDENCIES "${PACKAGE_PREFIX_DIR}/lib/cmake/vmb/configs/${DEP}.cmake")
                endforeach()
                list(APPEND VMB_FIND_FILES ${DEPENDENCIES} "${PACKAGE_PREFIX_DIR}/lib/cmake/vmb/configs/vmb_${INCLUDE_FILE_NAME}.cmake")
                set(VMB_COMPONENT_${_COMP}_FOUND True PARENT_SCOPE)
            elseif(${CMAKE_FIND_PACKAGE_NAME}_FIND_REQUIRED_${_COMP})
                _vmb_find_message(FATAL_ERROR "Unknown Vmb component required: ${_COMP}")
            else()
                set(VMB_COMPONENT_${_COMP}_FOUND False PARENT_SCOPE)
                _vmb_find_message(WARNING "Unknown Vmb component: ${_COMP}")
            endif()
        endforeach()
    endif()

    list(REMOVE_DUPLICATES VMB_FIND_FILES)
    
    # get the root directory of the installation
    get_filename_component(_VMB_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
    set(VMB_BINARY_DIRS "${PACKAGE_PREFIX_DIR}/bin" CACHE INTERNAL "Directory containing the Vmb dlls")

    set(_VMB_FIND_PACKAGE_IMPL_FILES ${VMB_FIND_FILES} PARENT_SCOPE)
    set(${CMAKE_FIND_PACKAGE_NAME}_FOUND 1 PARENT_SCOPE)
endfunction()

_vmb_find_package_impl()

foreach(INCLUDE IN LISTS _VMB_FIND_PACKAGE_IMPL_FILES)
    include(${INCLUDE})
endforeach()
