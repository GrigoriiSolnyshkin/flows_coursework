cmake_minimum_required(VERSION 3.22)
project(flows_coursework)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS_ASAN "-g -fsanitize=address,undefined -fno-sanitize-recover=all"
        CACHE STRING "Compiler flags in asan build"
        FORCE)

set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wextra -Werror"
        CACHE STRING "Compiler flags in debug build"
        FORCE)

include_directories(libraries/)

add_executable(flows-coursework main.cpp)
add_executable(unit-tests-run
        unit-tests/doctest_main.cpp
        unit-tests/link_cut_tests.cpp
        unit-tests/flows_utils_tests.cpp)
