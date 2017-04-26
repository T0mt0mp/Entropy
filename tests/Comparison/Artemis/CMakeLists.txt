cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
cmake_policy(VERSION 2.8)

project(ComparisonArtemis)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

set(PROJECT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

include_directories(${PROJECT_INCLUDE_DIR})
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${COMMON_INCLUDE})
include_directories(${EXTERN_DIR}/artemis/include/)

set(TESTS_SOURCES
        ${PROJECT_SOURCE_DIR}/Comp.cpp)
set(TESTS_HEADERS
        ${PROJECT_INCLUDE_DIR}/Comp.h)

add_executable(${PROJECT_NAME} ${TESTS_SOURCES} ${TESTS_HEADERS})
target_link_libraries(${PROJECT_NAME} ArtemisCpp ECS-FIT)
