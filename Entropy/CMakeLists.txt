cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
cmake_policy(VERSION 2.8)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR})

project(Entropy)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -std=c++1z -pedantic -Wall -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -std=c++1z -pedantic -Wall -Werror")

set(ENTROPY_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(ENTROPY_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(ENTROPY_INCLUDE ${ENTROPY_INCLUDE_DIR} PARENT_SCOPE)

include_directories(${ENTROPY_INCLUDE_DIR})

set(ENTROPY_HEADERS
        ${ENTROPY_INCLUDE_DIR}/Entropy/List.h
        ${ENTROPY_INCLUDE_DIR}/Entropy/SortedList.h
        ${ENTROPY_INCLUDE_DIR}/Entropy/Memory.h
        ${ENTROPY_INCLUDE_DIR}/Entropy/ComponentStorage.h
        ${ENTROPY_INCLUDE_DIR}/Entropy/Component.h
        ${ENTROPY_INCLUDE_DIR}/Entropy/ActionCache.h
        ${ENTROPY_INCLUDE_DIR}/Entropy/Entity.h
        ${ENTROPY_INCLUDE_DIR}/Entropy/EntityId.h
        ${ENTROPY_INCLUDE_DIR}/Entropy/EntityManager.h
        ${ENTROPY_INCLUDE_DIR}/Entropy/System.h
        ${ENTROPY_INCLUDE_DIR}/Entropy/Universe.h
        ${ENTROPY_INCLUDE_DIR}/Entropy/Types.h
        ${ENTROPY_INCLUDE_DIR}/Entropy/Entropy.h
        ${ENTROPY_INCLUDE_DIR}/Entropy/Assert.h
        ${ENTROPY_INCLUDE_DIR}/Entropy/Group.h
        ${ENTROPY_INCLUDE_DIR}/Entropy/Util.h
        )

set(ENTROPY_SOURCES
        ${ENTROPY_SOURCE_DIR}/Entropy/Component.cpp
        ${ENTROPY_SOURCE_DIR}/Entropy/Entity.cpp
        ${ENTROPY_SOURCE_DIR}/Entropy/EntityId.cpp
        ${ENTROPY_SOURCE_DIR}/Entropy/EntityManager.cpp
        ${ENTROPY_SOURCE_DIR}/Entropy/System.cpp
        ${ENTROPY_SOURCE_DIR}/Entropy/Universe.cpp
        ${ENTROPY_SOURCE_DIR}/Entropy/Group.cpp
        ${ENTROPY_SOURCE_DIR}/Entropy/Types.cpp
        ${ENTROPY_SOURCE_DIR}/Entropy/Util.cpp
        )

add_library(ENTROPY ${ENTROPY_SOURCES} ${ENTROPY_HEADERS})

# Testing
add_subdirectory(tests)
