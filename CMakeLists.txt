cmake_minimum_required(VERSION 3.26)
project(flows_coursework)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS_ASAN "-g -fsanitize=address,undefined -fno-sanitize-recover=all"
        CACHE STRING "Compiler flags in asan build"
        FORCE)

set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wextra -Werror"
        CACHE STRING "Compiler flags in debug build"
        FORCE)

add_executable(flows_coursework main.cpp)
