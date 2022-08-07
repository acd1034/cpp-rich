cmake_minimum_required(VERSION 3.12)
project(Iris
  VERSION 0.1.0
  LANGUAGES CXX
)
include(GNUInstallDirs)

# Creates a library Iris which is an interface (header files only)
add_library(Iris INTERFACE)
# If you want to use Iris prefer to link against Iris using this alias target
add_library(Iris::Iris ALIAS Iris)

# Specify build type ifnot
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
  message(STATUS "CMAKE_BUILD_TYPE not specified: Use Release by default.")
endif(NOT CMAKE_BUILD_TYPE)

# Determine whether this is a standalone project or included by other projects
set(IRIS_STANDALONE_PROJECT OFF)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(IRIS_STANDALONE_PROJECT ON)
endif()

# Project options
option(IRIS_INSTALL "Generate and install Iris target" ${IRIS_STANDALONE_PROJECT})
option(IRIS_TEST "Build and perform Iris tests" ${IRIS_STANDALONE_PROJECT})

# Setup include directory
add_subdirectory(include)

if(IRIS_INSTALL)
  install(
    TARGETS Iris
    EXPORT IrisConfig
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    # RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )
  install(
    DIRECTORY include/iris
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )
  # Make library importable by other projects
  install(
    EXPORT IrisConfig
    NAMESPACE Iris::
    FILE IrisConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Iris
    # EXPORT_LINK_INTERFACE_LIBRARIES
  )
endif()

if(IRIS_TEST)
  include(CTest)
  add_subdirectory(tests)
endif()
