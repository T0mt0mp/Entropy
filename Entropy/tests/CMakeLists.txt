cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
cmake_policy(VERSION 2.8)

project(Entropy_tests)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

set(PROJECT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

include_directories(${PROJECT_INCLUDE_DIR})
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${ENGINE_INCLUDE_DIR})

add_subdirectory(Misc)
add_subdirectory(Entropy)
add_subdirectory(Performance)
add_subdirectory(Sandbox)
