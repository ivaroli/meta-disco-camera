#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Vmb::CPP" for configuration "Release"
set_property(TARGET Vmb::CPP APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Vmb::CPP PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libVmbCPP.so"
  IMPORTED_SONAME_RELEASE "libVmbCPP.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS Vmb::CPP )
list(APPEND _IMPORT_CHECK_FILES_FOR_Vmb::CPP "${_IMPORT_PREFIX}/lib/libVmbCPP.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
