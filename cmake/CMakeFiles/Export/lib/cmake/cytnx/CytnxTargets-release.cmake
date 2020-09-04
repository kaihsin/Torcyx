#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Cytnx::cytnx" for configuration "Release"
set_property(TARGET Cytnx::cytnx APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Cytnx::cytnx PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libcytnx.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS Cytnx::cytnx )
list(APPEND _IMPORT_CHECK_FILES_FOR_Cytnx::cytnx "${_IMPORT_PREFIX}/lib/libcytnx.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
