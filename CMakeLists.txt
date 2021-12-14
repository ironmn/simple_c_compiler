cmake_minimum_required(VERSION 3.21)
project(simple_C_compiler)
# Locate GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
set(CMAKE_CXX_STANDARD 11)
configure_file(test/scanner_test.txt scanner_test.txt COPYONLY)
add_executable(simple_C_compiler gtest.cpp common.h token.h token.cpp lexer.h lexer.cpp error.h error.cpp compiler.h compiler.cpp)

target_link_libraries(simple_C_compiler ${GTEST_LIBRARIES} pthread)
