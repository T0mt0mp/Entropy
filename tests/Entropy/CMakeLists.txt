cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
cmake_policy(VERSION 2.8)

project(TestEntropy)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

set(PROJECT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

include_directories(${PROJECT_INCLUDE_DIR})
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${ENGINE_INCLUDE_DIR})

set(TESTS_SOURCES
        ${PROJECT_SOURCE_DIR}/TestUniverse.cpp
        ${PROJECT_SOURCE_DIR}/SecondModule.cpp
        ${PROJECT_SOURCE_DIR}/Tests.cpp)
set(TESTS_HEADERS
        ${PROJECT_INCLUDE_DIR}/TestUniverse.h
        ${PROJECT_INCLUDE_DIR}/SecondModule.h
        ${PROJECT_INCLUDE_DIR}/Tests.h)

add_executable(${PROJECT_NAME} ${TESTS_SOURCES} ${TESTS_HEADERS})
target_link_libraries(${PROJECT_NAME} ECS-FIT pthread)
