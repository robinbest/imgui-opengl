# 2024
# See https://github.com/coderefinery/cmake-workshop/blob/main/content/examples/testing
# an irrelavent test for github workflow

# project name and language
project(tests LANGUAGES CXX)

# example library
add_library(sum_integers sum_integers.cpp)

# main code
add_executable(sum_up main.cpp)
target_link_libraries(sum_up PRIVATE sum_integers)

# testing binary
add_executable(cpp_test test_sum.cpp)
target_link_libraries(cpp_test PRIVATE sum_integers)

# define tests
add_test(
  NAME cpp_test
  COMMAND $<TARGET_FILE:cpp_test>
)
