cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
cmake_policy(VERSION 2.8)

project(Comparison)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(external/anax EXCLUDE_FROM_ALL)
add_subdirectory(external/artemis EXCLUDE_FROM_ALL)

if ((${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 3.1) OR (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} EQUAL 3.1))
    set(ENTITYX_BUILD_SHARED false)
    set(ENTITYX_BUILD_TESTING false CACHE INTERNAL "" FORCE)
    add_subdirectory(external/entityx EXCLUDE_FROM_ALL)
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-std=c++1z -O3")
    set(CMAKE_CXX_FLAGS_DEBUG "")
    set(CMAKE_CXX_FLAGS_RELEASE "")
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "/O2")
    set(CMAKE_CXX_FLAGS_DEBUG "")
    set(CMAKE_CXX_FLAGS_RELEASE "")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "-std=c++1z -O3")
    set(CMAKE_CXX_FLAGS_DEBUG "")
    set(CMAKE_CXX_FLAGS_RELEASE "")
endif()

set (CMAKE_CXX_STANDARD 14)

set(EXTERN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external)
set(COMMON_INCLUDE
        ${ENGINE_INCLUDE}
        ${CMAKE_CURRENT_SOURCE_DIR}/external/common
        )

if ((${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 3.1) OR (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} EQUAL 3.1))
    add_subdirectory(Entityx)
endif()
add_subdirectory(Entropy)
add_subdirectory(Anax)
add_subdirectory(Artemis)
