cmake_minimum_required(VERSION 3.12)
project(main_tests CXX)

# ${CMAKE_PROJECT_NAME}: project name of the root CMakeLists.txt
# ${PROJECT_NAME}: project name of the current CMakeLists.txt
add_executable(${PROJECT_NAME}
  main.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  Iris::Iris
  IrisTestsConfig
  Catch2::Catch2WithMain
)

add_test(${PROJECT_NAME} ${PROJECT_NAME})
