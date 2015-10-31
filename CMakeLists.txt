# CMake Configuration File for hashmap.cpp

cmake_minimum_required (VERSION 2.8.11)

set(AUTHOR "Thomas Kenying Lau <thomklau@stanford.edu>")
project (HASHMAP_IMPLEMENTATION)
set(CMAKE_BUILD_TYPE Release)
add_executable(maptest maptest.cpp)