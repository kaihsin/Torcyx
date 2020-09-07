###
#  @file CMakeLists.txt
#  @license
#    Copyright (c) 2020-
#
#    This file is part of Torcyx.
#
#  @author Kai-Hsin Wu,
#  @date 2020-09-03
#  @since 0.1
#  
###

######################################################################
### CMAKE VERSION
######################################################################
cmake_minimum_required(VERSION 3.11)


## Pytorch vars:
execute_process (
COMMAND bash -c "python -c 'import torch;print(torch.utils.cmake_prefix_path)'"
OUTPUT_VARIABLE TORCH_CMAKE_PATH_C
)
string(REGEX REPLACE "\n$" "" TORCH_CMAKE_PATH_C "${TORCH_CMAKE_PATH_C}")
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${TORCH_CMAKE_PATH_C})
#message(STATUS "${TORCH_CMAKE_PATH_C}")
message(STATUS ${CMAKE_PREFIX_PATH})



## General setting
include(cmake/target_sources_local.cmake)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")
message(STATUS ${CMAKE_MODULE_PATH})
include(GNUInstallDirs)


######################################################################
### DISABLING IN-SOURCE BUILD
### (DO NOT MODIFY)
######################################################################
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR
    "
     In-source builds not allowed.\n
     Please\n
       1) make a build directory by:
            mkdir -p path/to/my/build/directory\n
       2) run CMake in that build directory:
            cd path/to/my/build/directory
            cmake path/to/my/src/directory

     You have to first remove the file \"CMakeCache.txt\" by:
         rm CMakeCache.txt
    "
  )
endif()
# Generate pif code
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

string(TOLOWER "${CMAKE_BUILD_TYPE}" cmake_build_type_tolower)
if(    NOT cmake_build_type_tolower STREQUAL "debug"
   AND NOT cmake_build_type_tolower STREQUAL "release"
   AND NOT cmake_build_type_tolower STREQUAL "relwithdebinfo")
  message(FATAL_ERROR "Unknown build type \"${CMAKE_BUILD_TYPE}\". Allowed values are Debug, Release, RelWithDebInfo (case-insensitive).")
endif()


######################################################################
# Version information
######################################################################
set(TORCYX_VERSION_MAJOR 0)
set(TORCYX_VERSION_MINOR 1)
set(TORCYX_VERSION_PATCH 0)
set(TORCYX_VERSION COUNT 0)
set(TORCYX_VERSION
  ${TORCYX_VERSION_MAJOR}.${TORCYX_VERSION_MINOR}.${TORCYX_VERSION_PATCH}
)



######################################################################
### COMMAND LINE OPTIONS
######################################################################

#option(BUILD_PYTHON "Build Python wrapper" OFF)
option(BUILD_DOC "Build API docuemntation" OFF)

######################################################################
### PROJECT
######################################################################
project(TORCYX VERSION ${TORCYX_VERSION} LANGUAGES CXX C )

add_library(torcyx STATIC)
set_property(TARGET torcyx PROPERTY C_VISIBILITY_PRESET hidden)
set_property(TARGET torcyx PROPERTY VISIBILITY_INLINES_HIDDEN ON)
target_include_directories(torcyx
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_compile_features(torcyx PUBLIC cxx_std_14)
## Using Cytnx's 
#target_include_directories(torcyx SYSTEM
#    PUBLIC ${Boost_INCLUDE_DIRS}
#)



add_subdirectory(src)
message(STATUS ${COMPILE_DEFINITIONS})


## pytorch:
find_package(Torch REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wformat=0 -Wno-c++11-narrowing -w")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}") 
message(STATUS "pytorch: ${TORCH_INSTALL_PREFIX}")
#set_property(TARGET torcyx PROPERTY CXX_STANDARD 14)

## cytnx flags:
execute_process (
COMMAND bash -c "python -c 'import sys,os;sys.path.append(os.path.expanduser(\"${CYTNX_PATH}\"));import cytnx;print(cytnx.__cpp_include__)'"
OUTPUT_VARIABLE CYTNX_INC_PATH_C
)
string(REGEX REPLACE "\n$" "" CYTNX_INC_PATH "${CYTNX_INC_PATH_C}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${CYTNX_INC_PATH} -pthread")
message(STATUS ${CMAKE_CXX_FLAGS})
#--------
execute_process (
COMMAND bash -c "python -c 'import sys,os;sys.path.append(os.path.expanduser(\"${CYTNX_PATH}\"));import cytnx;print(cytnx.__cpp_lib__)'"
OUTPUT_VARIABLE CYTNX_LIB_C
)
string(REGEX REPLACE "\n$" "" CYTNX_LIB "${CYTNX_LIB_C}")
set(CYTNX_LIBRARIES "${CYTNX_LIB}/libcytnx.a")
execute_process (
COMMAND bash -c "python -c 'import sys,os;sys.path.append(os.path.expanduser(\"${CYTNX_PATH}\"));import cytnx;print(cytnx.__cpp_linkflags__)'"
OUTPUT_VARIABLE CYTNX_LINK_C
)
string(REGEX REPLACE "\n$" "" CYTNX_LINK "${CYTNX_LINK_C}")
string(REGEX REPLACE " $" "" CYTNX_LINK_C "${CYTNX_LINK}")
string(REPLACE " " ";" CYTNX_LINK "${CYTNX_LINK_C}")
set(CYTNX_LIBRARIES "${CYTNX_LIBRARIES};${CYTNX_LINK}")
message(STATUS "${CYTNX_LIBRARIES}")

target_link_libraries(torcyx ${TORCH_LIBRARIES} ${TORCH_INSTALL_PREFIX}/lib/libtorch_python.so)
target_link_libraries(torcyx ${CYTNX_LIBRARIES})
set_property(TARGET torcyx PROPERTY CXX_STANDARD 14)

## python:

find_package(PythonInterp REQUIRED)
find_package(pybind11 REQUIRED)
if( NOT PYBIND11_FOUND)
    include(FetchContent)
    FetchContent_Declare(
      pybind11_sources
      GIT_REPOSITORY https://github.com/pybind/pybind11.git
      GIT_TAG v2.4.3
      )

    FetchContent_GetProperties(pybind11_sources)

    if(NOT pybind11_sources_POPULATED)
      FetchContent_Populate(pybind11_sources)

      add_subdirectory(
        ${pybind11_sources_SOURCE_DIR}
        ${pybind11_sources_BINARY_DIR}
        )
    endif()
endif()
pybind11_add_module(pytorcyx SHARED pybind/torcyx.cpp)
target_link_libraries(pytorcyx PRIVATE torcyx) 
set_property(TARGET pytorcyx PROPERTY CXX_STANDARD 14)
set_target_properties(pytorcyx PROPERTIES OUTPUT_NAME torcyx)


######################################################################
### Build Documents
######################################################################
IF(BUILD_DOC)
  FIND_PACKAGE(Doxygen REQUIRED)
  if(DOXYGEN_FOUND)
    configure_file(cytnx_Doxyfile.in cytnxDoxyfile)
    add_custom_target(doc ALL
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/cytnxDoxyfile
     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
     COMMENT "Generating API documentation with Doxygen" VERBATIM
     )
  endif(DOXYGEN_FOUND)
ENDIF()

######################################################################
### INSTALL
######################################################################

set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/torcyx)
INSTALL(TARGETS torcyx EXPORT torcyx_targets
        LIBRARY
          DESTINATION ${CMAKE_INSTALL_LIBDIR}
          COMPONENT libraries
        ARCHIVE
          DESTINATION ${CMAKE_INSTALL_LIBDIR}
          COMPONENT libraries
        PUBLIC_HEADER
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
          COMPONENT Development
      )

## python:

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/torcyx
    DESTINATION ${CMAKE_INSTALL_PREFIX}
    )
INSTALL(TARGETS pytorcyx EXPORT pytorcyx_targets
        LIBRARY
          DESTINATION ${CMAKE_INSTALL_PREFIX}/torcyx
          COMPONENT libraries
      )


#install(EXPORT torcyx_targets
#        FILE TorcyxTargets.cmake
#        NAMESPACE torcyx::
#        DESTINATION ${INSTALL_CONFIGDIR}
#        )
install(DIRECTORY include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        COMPONENT headers
        FILES_MATCHING PATTERN 		"*.h*")


#Create a ConfigVersion.cmake file
#include(CMakePackageConfigHelpers)
#write_basic_package_version_file(
#        ${CMAKE_CURRENT_BINARY_DIR}/TorcyxCofigVersion.cmake
#        VERSION ${TORCYX_VERSION}
#        COMPATIBILITY AnyNewerVersion
#)

#configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/TorcyxConfig.cmake.in
#        ${CMAKE_CURRENT_BINARY_DIR}/TorcyxConfig.cmake
#        INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
#        )
#install(FILES
#        ${CMAKE_CURRENT_BINARY_DIR}/TorcyxConfig.cmake
#        ${CMAKE_CURRENT_BINARY_DIR}/TorcyxConfigVersion.cmake
#        DESTINATION ${INSTALL_CONFIGDIR}
#        )

##############################################
## Exporting from the build tree

#export(EXPORT torcyx_targets FILE ${CMAKE_CURRENT_BINARY_DIR}/TorcyxTargets.cmake NAMESPACE torcyx::)

export(PACKAGE Torcyx)


######################################################################
### INSTALL DOCUMENTATION
######################################################################
if(DOXYGEN_FOUND)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docs/
	DESTINATION docs
        COMPONENT documentation
)
endif()

###########
#  test
###########
##testing:
if(BUILD_TEST)
  project(test)
  add_executable(test ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)
  target_link_libraries(test torcyx)
  #target_link_libraries(test PRIVATE ${CYTNX_LIBRARIES})
  set_property(TARGET test PROPERTY CXX_STANDARD 14)
endif()

set(libname "${CMAKE_SHARED_LIBRARY_PREFIX}torcyx.${TORCYX_VERSION}${CMAKE_SHARED_LIBRARY_SUFFIX}")

# create a file that contain all the link flags:
FILE(WRITE "${CMAKE_BINARY_DIR}/linkflags.tmp" "" "")

######################################################################
# Information Messages
######################################################################
message(STATUS "")
message(STATUS "------------------------------------------------------------------------")
message(STATUS "  Torcyx, ML with cytnx and pytorch")
message(STATUS "------------------------------------------------------------------------")
message(STATUS "")
message(STATUS " Version: ${TORCYX_VERSION}")
message(STATUS " Generator: ${CMAKE_GENERATOR}")
message(STATUS " Build Target: ${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS " Installation Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS " CXX Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS " CXX Flags: ${CMAKE_CXX_FLAGS}")
message(STATUS " Link libraries: ${TORCH_LIBRARIES}")
#FILE(APPEND "${CMAKE_BINARY_DIR}/linkflags.tmp" "${LAPACK_LIBRARIES}\n" "")


message(STATUS " Build Python Wrapper: YES")
message(STATUS "  - Python Excutable  : ${PYTHON_EXECUTABLE}")
message(STATUS "  - Python Headers    : ${PYTHON_INCLUDE_DIR}")
message(STATUS "  - Python Library    : ${PYTHON_LIBRARY}")

FILE(APPEND "${CMAKE_BINARY_DIR}/linkflags.tmp" "${TORCH_LIBRARIES}\n" "") 
FILE(APPEND "${CMAKE_BINARY_DIR}/linkflags.tmp" "${CYTNX_LIBRARIES}\n" "")
FILE(WRITE "${CMAKE_BINARY_DIR}/cytnxpath.tmp" "${CYTNX_PATH}" "")

if(BUILD_DOC)
  message( STATUS " Build Documentation: YES")
  message( STATUS "  - Doxygen: ${DOXYGEN_EXECUTABLE}")
else()
  message( STATUS " Build Documentation: NO")
endif()

message(STATUS "")
message(STATUS "")
message(STATUS "")


install(FILES ${CMAKE_BINARY_DIR}/linkflags.tmp DESTINATION ${CMAKE_INSTALL_PREFIX}/torcyx)
install(FILES ${CMAKE_BINARY_DIR}/cytnxpath.tmp DESTINATION ${CMAKE_INSTALL_PREFIX}/torcyx)




#if (BUILD_PACKAGES)
#include("Package.cmake")
#include(CPack)
#endif()
