cmake_minimum_required(VERSION 3.22.0)

set(PROJECT raytracing)
project(${PROJECT})

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

include(cmake/StandardProjectSettings.cmake)
include(cmake/PreventInSourceBuilds.cmake)

# Link this 'library' to set the C++ standard /compile-time options requested
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_20)
set(CMAKE_CXX_EXTENSIONS OFF) # Forces use of -std=c++20 instead of -std=gnu++20

if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
  option(ENABLE_BUILD_WITH_TIME_TRACE "Enable -ftime-trace to generate time tracing .json files on clang" OFF)
  if(ENABLE_BUILD_WITH_TIME_TRACE)
    target_compile_options(project_options INTERFACE -ftime-trace)
  endif()
endif()

# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
add_library(project_warnings INTERFACE)

# standard compiler warnings
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

# standard compiler warnings
include(cmake/CompilerDefines.cmake)
set_project_defines(project_options)

# sanitizer options if supported by compiler
include(cmake/Sanitizers.cmake)
enable_sanitizers(project_options)

# allow for static analysis options
include(cmake/StaticAnalyzers.cmake)

# Global options
option(ENABLE_TESTING "Enable Test Builds" ON)
option(BUILD_DOCS "Build Documentation" OFF)

# Define target names
set(PROJECT_LIB ${PROJECT})
set(PROJECT_EXE ${PROJECT}.bin)
set(PROJECT_TEST ${PROJECT}.test)

# Build library and executable
add_subdirectory(src)

# Build and run tests
if(ENABLE_TESTING)
  enable_testing()
  add_subdirectory(tests)
endif()

# Documentation
if(BUILD_DOCS)
  add_subdirectory(docs)
endif()

