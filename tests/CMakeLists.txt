cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
cmake_policy(VERSION 2.8)

project(tests)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

set(PROJECT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

include_directories(${PROJECT_INCLUDE_DIR})
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${ENGINE_INCLUDE_DIR})

add_subdirectory(List)
add_subdirectory(Common)
add_subdirectory(DataStruct)
add_subdirectory(BPTree)
add_subdirectory(BPTreePure)
add_subdirectory(Profiler)
add_subdirectory(Tsc)
add_subdirectory(Misc)
