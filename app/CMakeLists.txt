cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
cmake_policy(VERSION 2.8)

project(gameTest)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

set(PROJECT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

include_directories(${PROJECT_INCLUDE_DIR})
include_directories(${PROJECT_SOURCE_DIR})

include_directories(${ENGINE_INCLUDE})
include_directories(${ENTROPY_INCLUDE})

set(APP_SOURCES
        ${PROJECT_SOURCE_DIR}/ECS.cpp
        ${PROJECT_SOURCE_DIR}/App.cpp)
set(APP_HEADERS
        ${PROJECT_INCLUDE_DIR}/Types.h
        ${PROJECT_INCLUDE_DIR}/ECS.h
        ${PROJECT_INCLUDE_DIR}/GLSLProgram.h
        ${PROJECT_INCLUDE_DIR}/Camera.h
        ${PROJECT_INCLUDE_DIR}/Timer.h
        ${PROJECT_INCLUDE_DIR}/Keyboard.h
        ${PROJECT_INCLUDE_DIR}/Mouse.h
        ${PROJECT_INCLUDE_DIR}/Gamepad.h
        ${PROJECT_INCLUDE_DIR}/App.h)

add_executable(${PROJECT_NAME} ${APP_SOURCES} ${APP_HEADERS})
target_link_libraries(${PROJECT_NAME} GL GLEW glfw ECS-FIT)

add_custom_target(
        copyShaders
        ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        #"${PROJECT_SOURCE_DIR}/shaders" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders"
        "${PROJECT_SOURCE_DIR}/shaders" "${EXECUTABLE_OUTPUT_PATH}/shaders"
)

add_dependencies(${PROJECT_NAME} copyShaders)
