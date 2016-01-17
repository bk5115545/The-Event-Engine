# FindGLEW - attempts to locate the GL Extension Wrangler (GLEW) library
#
# This module defines the following variables (on success):
#   GLEW_INCLUDE_DIRS  - where to find glew.h
#   GLEW_LIBRARIES     - GLEW library
#   GLEW_MX_LIBRARIES  - GLEW multi context library
#   GLEW_FOUND         - if the library was successfully located
#
# It is trying a few standard installation locations, but can be customized
# with the following variables:
#   GLEW_ROOT_DIR     - root directory of a GLEW installation
#                       Headers are expected to be found in:
#                       <GLEW_ROOT_DIR>/include/GL/glew.hpp
#                       This variable can either be a cmake or environment
#                       variable. Note however that changing the value
#                       of the environment varible will NOT result in
#                       re-running the header search and therefore NOT
#                       adjust the variables set by this module.

#=============================================================================
# Copyright 2012 Carsten Neumann
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

# default search dirs
set(_glew_HEADER_SEARCH_DIRS
    "/usr/include"
    "/usr/local/include")

set(_glew_LIBRARY_SEARCH_DIRS
    "/usr/lib"
    "/usr/local/lib")

# check environment variable
set(_glew_env_ROOT_DIR "$ENV{GLEW_ROOT_DIR}")

if(NOT GLEW_ROOT_DIR AND _glew_env_ROOT_DIR)
    set(GLEW_ROOT_DIR "${_glew_env_ROOT_DIR}")
endif()

# put user specified location at beginning of search
if(GLEW_ROOT_DIR)
    set(_glew_HEADER_SEARCH_DIRS
        "${GLEW_ROOT_DIR}/include"
        ${_glew_HEADER_SEARCH_DIRS})

    set(_glew_LIBRARY_SEARCH_DIRS
        "${GLEW_ROOT_DIR}/lib"
        "${GLEW_ROOT_DIR}/lib64"
        ${_glew_LIBRARY_SEARCH_DIRS})
endif()

# locate header
find_path(GLEW_INCLUDE_DIR "GL/glew.h"
    PATHS ${_glew_HEADER_SEARCH_DIRS})

# locate library
find_library(GLEW_LIBRARY NAMES "GLEW" "glew32"
    PATHS ${_glew_LIBRARY_SEARCH_DIRS})

find_library(GLEW_MX_LIBRARY NAMES "GLEWmx" "glew32mx"
    PATHS ${_glew_LIBRARY_SEARCH_DIRS})

include("FindPackageHandleStandardArgs")
find_package_handle_standard_args(GLEW DEFAULT_MSG
    GLEW_INCLUDE_DIR GLEW_LIBRARY)

if(GLEW_FOUND)
    set(GLEW_INCLUDE_DIRS "${GLEW_INCLUDE_DIR}")
    set(GLEW_LIBRARIES    "${GLEW_LIBRARY}")
    set(GLEW_MX_LIBRARIES "${GLEW_MX_LIBRARY}")

    if(NOT GLEW_FIND_QUIETLY)
        message(STATUS "GLEW_INCLUDE_DIR... ${GLEW_INCLUDE_DIR}")
        message(STATUS "GLEW_LIBRARIES..... ${GLEW_LIBRARIES}")
        message(STATUS "GLEW_MX_LIBRARIES.. ${GLEW_MX_LIBRARIES}")
    endif()
endif()

include("FeatureSummary")
set_package_properties(GLEW PROPERTIES
    DESCRIPTION "GL Extension Wrangler Library"
    URL "http://glew.sourceforge.net"
)
