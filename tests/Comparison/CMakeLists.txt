cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
cmake_policy(VERSION 2.8)

project(Comparison)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(external/anax)
add_subdirectory(external/artemis)

set(ENTITYX_BUILD_SHARED 0)
set(ENTITYX_BUILD_TESTING 0)
add_subdirectory(external/entityx)

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

add_subdirectory(Entityx)
add_subdirectory(Entropy)
add_subdirectory(Anax)
add_subdirectory(Artemis)
