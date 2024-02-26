#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Vmb::C" for configuration "Release"
set_property(TARGET Vmb::C APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Vmb::C PROPERTIES
  IMPORTED_LINK_DEPENDENT_LIBRARIES_RELEASE "GenApi"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libVmbC.so"
  IMPORTED_SONAME_RELEASE "libVmbC.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS Vmb::C )
list(APPEND _IMPORT_CHECK_FILES_FOR_Vmb::C "${_IMPORT_PREFIX}/lib/libVmbC.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
