cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
cmake_policy(VERSION 2.8)

project(gameTest)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

set(PROJECT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

include_directories(${PROJECT_INCLUDE_DIR})
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${ENGINE_INCLUDE_DIR})

set(APP_SOURCES
        ${PROJECT_SOURCE_DIR}/App.cpp)
set(APP_HEADERS
        ${PROJECT_INCLUDE_DIR}/App.h)

add_executable(${PROJECT_NAME} ${APP_SOURCES} ${APP_HEADERS})
target_link_libraries(${PROJECT_NAME} ECS-FIT)
