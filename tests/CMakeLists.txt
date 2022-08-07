cmake_minimum_required(VERSION 3.12)
project(IrisTests
  LANGUAGES CXX
)
include(CTest)

# Import the globally installed Iris ifCMake has been started independently in
# this directory with tests
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  find_package(Iris REQUIRED)
endif()

# This interface adds compile options to how the tests are run
add_library(IrisTestsConfig INTERFACE)
target_compile_features(IrisTestsConfig INTERFACE cxx_std_20)
set_target_properties(IrisTestsConfig PROPERTIES
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
)
target_compile_options(IrisTestsConfig INTERFACE
  -fno-strict-aliasing
  -Wall
  -Wcast-align
  -Wconversion
  -Wctor-dtor-privacy
  # -Werror
  -Wextra
  -Wpedantic
  -Wshadow
  -Wsign-conversion
  -Wno-deprecated-declarations # Allow tests for [[deprecated]] elements
  $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:
    -Weverything
    -Wno-c++98-compat
    -Wno-c++98-compat-pedantic
    -Wno-missing-braces
    -Wno-covered-switch-default # GTest
    -Wno-global-constructors # GTest
    -Wno-missing-prototypes
    -Wno-padded
    -Wno-unknown-attributes
    -Wno-used-but-marked-unused # GTest EXPECT_DEATH
    -Wno-weak-vtables
  >
  # $<$<CXX_COMPILER_ID:Clang>:
  #   $<$<AND:$<VERSION_GREATER:$<CXX_COMPILER_VERSION>,4.99>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,6>>:
  #     $<$<EQUAL:${IRIS_CXX_STANDARD},17>:-Wno-undefined-func-template>
  #   >
  # >
  # $<$<CXX_COMPILER_ID:AppleClang>:
  #   $<$<AND:$<VERSION_GREATER:$<CXX_COMPILER_VERSION>,9.1>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,10>>:
  #     $<$<EQUAL:${IRIS_CXX_STANDARD},17>:-Wno-undefined-func-template>
  #   >
  # >
  $<$<CXX_COMPILER_ID:GNU>:
    -Wdouble-promotion # float implicit to double
    -Wlogical-op # suspicious uses of logical operators
    $<$<NOT:$<VERSION_LESS:$<CXX_COMPILER_VERSION>,6>>:
      -Wduplicated-cond # duplicated if-else conditions
      -Wmisleading-indentation
      -Wnull-dereference
    >
    $<$<NOT:$<VERSION_LESS:$<CXX_COMPILER_VERSION>,7>>:
      -Wduplicated-branches # identical if-else branches
    >
  >
)

Include(FetchContent)
FetchContent_Declare(Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.0.1)
FetchContent_MakeAvailable(Catch2)

add_subdirectory(main)
