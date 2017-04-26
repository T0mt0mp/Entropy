cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
cmake_policy(VERSION 2.8)

project(tests)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(Misc)
add_subdirectory(Entropy)
add_subdirectory(Parallel)
add_subdirectory(Performance)
add_subdirectory(Sandbox)
add_subdirectory(Comparison)
