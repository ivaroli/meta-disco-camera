cmake_minimum_required(VERSION 3.0)

project(VmbCPPExamples)

set(VMB_CPP_EXAMPLES_LIST IGNORE CACHE STRING
    "a semicolon separated list of examples to configure; takes precedence over other settings to enable or disable examples"
)

set(VMB_CPP_ALL_EXAMPLES)

# function takes the directory and optionally aliases other than the directory the example can be refered by
function(vmb_cpp_example DIRECTORY)
    set(VMB_CPP_EXAMPLE_IGNORE_${DIRECTORY} False CACHE BOOL "Ignore the ${DIRECTORY} example; VMB_CPP_EXAMPLES_LIST takes precedence over this property")
    foreach(_ALIAS IN LISTS DIRECTORY ARGN)
        set(VMB_CPP_ALIAS_${_ALIAS} ${DIRECTORY} PARENT_SCOPE)
    endforeach()
    if (NOT VMB_CPP_EXAMPLE_IGNORE_${DIRECTORY})
        set(VMB_CPP_ALL_EXAMPLES ${VMB_CPP_ALL_EXAMPLES} ${DIRECTORY} PARENT_SCOPE)
    endif()
endfunction()

# Actual examples list
vmb_cpp_example(AsynchronousGrab)
vmb_cpp_example(SynchronousGrab)
vmb_cpp_example(ChunkAccess)
vmb_cpp_example(ListCameras)

# overwrite list of examples set based on individual ignores
if(VMB_CPP_EXAMPLES_LIST)
    set(VMB_CPP_ALL_EXAMPLES)
    foreach(_EXAMPLE IN LISTS VMB_CPP_EXAMPLES_LIST)
        set(_DIR ${VMB_CPP_ALIAS_${_EXAMPLE}})
        if (NOT _DIR)
            message(FATAL_ERROR "${_EXAMPLE} found in VMB_CPP_EXAMPLES_LIST is not a known example")
        endif()
    endforeach()
endif()

# finally add the necessary subdirectories
list(REMOVE_DUPLICATES VMB_CPP_ALL_EXAMPLES)

if (NOT VMB_CPP_ALL_EXAMPLES)
    message(FATAL_ERROR "no active examples")
endif()

foreach(_EXAMPLE IN LISTS VMB_CPP_ALL_EXAMPLES)
    add_subdirectory(${_EXAMPLE})
endforeach()
