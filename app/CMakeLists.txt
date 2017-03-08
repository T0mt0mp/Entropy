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

# Check the GLM submodule.
set(GLM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/glm")
if(NOT EXISTS "${GLM_DIR}/.git")
    message(WARNING "GLM submodule not initialized, initializing...")
    execute_process(COMMAND git submodule update --init -- external/glm WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

# Check the GLFW submodule.
set(GLFW_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/glfw")
if(NOT EXISTS "${GLFW_DIR}/.git")
    message(WARNING "GLFW submodule not initialized, initializing...")
    execute_process(COMMAND git submodule update --init -- external/glfw WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

# Check the GLEW submodule.
set(GLEW_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/glew_prepared")
#if(NOT EXISTS "${GLEW_DIR}/.git")
    #message(WARNING "GLEW submodule not initialized, initializing...")
    #execute_process(COMMAND git submodule update --init -- external/glew WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    #execute_process(COMMAND make extensions WORKING_DIRECTORY ${GLEW_DIR})
#endif()

# We don't want any of these.
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(${GLFW_DIR})

include_directories(
        ${GLFW_DIR}/include
        ${GLFW_DIR}/deps
        ${GLM_DIR}
        ${GLEW_DIR}/include
)

add_library(glew_static STATIC
        ${GLEW_DIR}/src/glew.c
)
add_definitions(
        -DGLEW_STATIC
)

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
target_link_libraries(${PROJECT_NAME} ECS-FIT GL glew_static glfw)

add_custom_target(
        copyShaders
        ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        #"${PROJECT_SOURCE_DIR}/shaders" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders"
        "${PROJECT_SOURCE_DIR}/shaders" "${EXECUTABLE_OUTPUT_PATH}/shaders"
)

add_dependencies(${PROJECT_NAME} copyShaders)
