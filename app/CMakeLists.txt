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

if(WIN32)
	set(GL_LIB opengl32.lib)
else()
	set(GL_LIB GL)
endif()


set(APP_SOURCES
        ${PROJECT_SOURCE_DIR}/ECS.cpp
        ${PROJECT_SOURCE_DIR}/App.cpp
        ${PROJECT_SOURCE_DIR}/Keyboard.cpp
        ${PROJECT_SOURCE_DIR}/Mouse.cpp
        ${PROJECT_SOURCE_DIR}/Gamepad.cpp
        ${PROJECT_SOURCE_DIR}/Cube.cpp
        ${PROJECT_SOURCE_DIR}/TextS.cpp
        ${PROJECT_SOURCE_DIR}/Font.cpp
        ${PROJECT_SOURCE_DIR}/FontFile.cpp
        ${PROJECT_SOURCE_DIR}/FontParser.cpp
        ${PROJECT_SOURCE_DIR}/Tables.cpp)
set(APP_HEADERS
        ${PROJECT_INCLUDE_DIR}/Types.h
        ${PROJECT_INCLUDE_DIR}/ECS.h
        ${PROJECT_INCLUDE_DIR}/App.h
        ${PROJECT_INCLUDE_DIR}/GLSLProgram.h
        ${PROJECT_INCLUDE_DIR}/Camera.h
        ${PROJECT_INCLUDE_DIR}/Timer.h
        ${PROJECT_INCLUDE_DIR}/Keyboard.h
        ${PROJECT_INCLUDE_DIR}/Mouse.h
        ${PROJECT_INCLUDE_DIR}/Gamepad.h
        ${PROJECT_INCLUDE_DIR}/Cube.h
        ${PROJECT_INCLUDE_DIR}/MovementS.h
        ${PROJECT_INCLUDE_DIR}/MovementS.inl
        ${PROJECT_INCLUDE_DIR}/RotationS.h
        ${PROJECT_INCLUDE_DIR}/RotationS.inl
        ${PROJECT_INCLUDE_DIR}/TransformS.h
        ${PROJECT_INCLUDE_DIR}/TransformS.inl
        ${PROJECT_INCLUDE_DIR}/RenderS.h
        ${PROJECT_INCLUDE_DIR}/RenderS.inl
        ${PROJECT_INCLUDE_DIR}/TextS.h
        ${PROJECT_INCLUDE_DIR}/Font.h
        ${PROJECT_INCLUDE_DIR}/FontFile.h
        ${PROJECT_INCLUDE_DIR}/FontFile.inl
        ${PROJECT_INCLUDE_DIR}/FontParser.h
        ${PROJECT_INCLUDE_DIR}/Tables.h)

add_executable(${PROJECT_NAME} ${APP_SOURCES} ${APP_HEADERS})
target_link_libraries(${PROJECT_NAME} ECS-FIT ENTROPY ${GL_LIB} glew_static glfw)

if(MSVC)
    add_custom_target(
            copyResources
            ALL
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            #"${PROJECT_SOURCE_DIR}/shaders" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders"
            "${CMAKE_CURRENT_SOURCE_DIR}/res" "${EXECUTABLE_OUTPUT_PATH}/Release/res"
    )
else()
    add_custom_target(
            copyResources
            ALL
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            #"${PROJECT_SOURCE_DIR}/shaders" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders"
            "${CMAKE_CURRENT_SOURCE_DIR}/res" "${EXECUTABLE_OUTPUT_PATH}/res"
    )
endif()

add_dependencies(${PROJECT_NAME} copyResources)
