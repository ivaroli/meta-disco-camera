#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Vmb::ImageTransform" for configuration "Release"
set_property(TARGET Vmb::ImageTransform APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Vmb::ImageTransform PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libVmbImageTransform.so"
  IMPORTED_SONAME_RELEASE "libVmbImageTransform.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS Vmb::ImageTransform )
list(APPEND _IMPORT_CHECK_FILES_FOR_Vmb::ImageTransform "${_IMPORT_PREFIX}/lib/libVmbImageTransform.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
