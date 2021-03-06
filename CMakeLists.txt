cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
cmake_policy(VERSION 2.8)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR})

project(ECS-FIT)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++1z -pedantic -Wall -Werror")

if(CMAKE_COMPILER_IS_GNUCXX)
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -pedantic -Wall -Werror")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
	set(THREAD_LIB pthread)
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS_DEBUG "-Zi -DEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2")
	set(THREAD_LIB "")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -pedantic -Wall -Werror")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
	set(THREAD_LIB pthread)
endif()

set (CMAKE_CXX_STANDARD 14)

set(ECS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(ECS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_subdirectory(external)
add_subdirectory(Entropy)

set(ENGINE_INCLUDE
        ${ECS_INCLUDE_DIR}
        ${ENTROPY_INCLUDE})

# Engine library

include_directories(${ECS_INCLUDE_DIR})

set(ECS_HEADERS
        # Math
        ${ECS_INCLUDE_DIR}/math/Math.h

        # Memory structures
        ${ECS_INCLUDE_DIR}/mem/List.h
        ${ECS_INCLUDE_DIR}/mem/DefaultAllocator.h
        ${ECS_INCLUDE_DIR}/mem/BPTree.h
        ${ECS_INCLUDE_DIR}/mem/BPTreeConfig.h

        # Testing utilities
        ${ECS_INCLUDE_DIR}/testing/HashNode.h
        ${ECS_INCLUDE_DIR}/testing/Profiler.h
        ${ECS_INCLUDE_DIR}/testing/PrintCrawler.h
        ${ECS_INCLUDE_DIR}/testing/TestBench.h
        ${ECS_INCLUDE_DIR}/testing/TestBed.h
        ${ECS_INCLUDE_DIR}/testing/TestUnit.h

        # Utility code
        ${ECS_INCLUDE_DIR}/util/Assert.h
        ${ECS_INCLUDE_DIR}/util/Threading.h
        ${ECS_INCLUDE_DIR}/util/Types.h
        )

set(ECS_SOURCES
        # Math
        ${ECS_SOURCE_DIR}/math/Math.cpp

        # Memory structures

        # Testing utilities
        ${ECS_SOURCE_DIR}/testing/Profiler.cpp
        ${ECS_SOURCE_DIR}/testing/TestBench.cpp
        ${ECS_SOURCE_DIR}/testing/TestBed.cpp
        ${ECS_SOURCE_DIR}/testing/TestUnit.cpp

        # Utility code
        ${ECS_SOURCE_DIR}/util/Threading.cpp
        )

add_library(ECS-FIT ${ECS_SOURCES} ${ECS_HEADERS})
target_link_libraries(ECS-FIT ENTROPY ${THREAD_LIB})

# Application

add_subdirectory(app)

# Testing

add_subdirectory(tests)

set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)

