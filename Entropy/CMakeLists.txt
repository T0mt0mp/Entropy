cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
cmake_policy(VERSION 2.8)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR})

project(Entropy C CXX)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -std=c++1z -pedantic -Wall -Werror")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -std=c++1z -pedantic -Wall -Werror")

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -pedantic -Wall -Werror")
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    #set(CMAKE_CXX_FLAGS_DEBUG "-g -Og")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS_DEBUG "-Zi -DEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -pedantic -Wall -Werror")
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

set (CMAKE_CXX_STANDARD 14)

set(ENTROPY_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(ENTROPY_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(ENTROPY_INCLUDE ${ENTROPY_INCLUDE_DIR} PARENT_SCOPE)

include_directories(${ENTROPY_INCLUDE_DIR})

set(ENTROPY_HEADERS
        ${ENTROPY_INCLUDE_DIR}/Entropy/List.h
        ${ENTROPY_INCLUDE_DIR}/Entropy/List.inl
        ${ENTROPY_INCLUDE_DIR}/Entropy/SortedList.h
        ${ENTROPY_INCLUDE_DIR}/Entropy/SortedList.inl
        ${ENTROPY_INCLUDE_DIR}/Entropy/Memory.h
        ${ENTROPY_INCLUDE_DIR}/Entropy/ComponentStorage.h
        ${ENTROPY_INCLUDE_DIR}/Entropy/ComponentStorage.inl
        ${ENTROPY_INCLUDE_DIR}/Entropy/ComponentManager.h
        ${ENTROPY_INCLUDE_DIR}/Entropy/ComponentManager.inl
        ${ENTROPY_INCLUDE_DIR}/Entropy/ChangeSet.h
        ${ENTROPY_INCLUDE_DIR}/Entropy/ChangeSet.inl
        ${ENTROPY_INCLUDE_DIR}/Entropy/ActionsCache.h
        ${ENTROPY_INCLUDE_DIR}/Entropy/ActionsCache.inl
        ${ENTROPY_INCLUDE_DIR}/Entropy/Entity.h
        ${ENTROPY_INCLUDE_DIR}/Entropy/Entity.inl
        ${ENTROPY_INCLUDE_DIR}/Entropy/EntityId.h
        ${ENTROPY_INCLUDE_DIR}/Entropy/EntityId.inl
        ${ENTROPY_INCLUDE_DIR}/Entropy/EntityMetadata.h
        ${ENTROPY_INCLUDE_DIR}/Entropy/EntityMetadata.inl
        ${ENTROPY_INCLUDE_DIR}/Entropy/EntityManager.h
        ${ENTROPY_INCLUDE_DIR}/Entropy/EntityManager.inl
        ${ENTROPY_INCLUDE_DIR}/Entropy/SystemManager.h
        ${ENTROPY_INCLUDE_DIR}/Entropy/SystemManager.inl
        ${ENTROPY_INCLUDE_DIR}/Entropy/Universe.h
        ${ENTROPY_INCLUDE_DIR}/Entropy/Universe.inl
        ${ENTROPY_INCLUDE_DIR}/Entropy/Types.h
        ${ENTROPY_INCLUDE_DIR}/Entropy/Entropy.h
        ${ENTROPY_INCLUDE_DIR}/Entropy/Assert.h
        ${ENTROPY_INCLUDE_DIR}/Entropy/EntityGroup.h
        ${ENTROPY_INCLUDE_DIR}/Entropy/EntityGroup.inl
        ${ENTROPY_INCLUDE_DIR}/Entropy/GroupManager.h
        ${ENTROPY_INCLUDE_DIR}/Entropy/GroupManager.inl
        ${ENTROPY_INCLUDE_DIR}/Entropy/Util.h
        ${ENTROPY_INCLUDE_DIR}/Entropy/Util.inl
        )

set(ENTROPY_SOURCES
        )

#add_library(ENTROPY STATIC ${ENTROPY_SOURCES} ${ENTROPY_HEADERS})
if(COMMAND target_sources)
    add_library(ENTROPY INTERFACE)
    target_sources(ENTROPY INTERFACE ${ENTROPY_HEADERS})
    target_include_directories(ENTROPY INTERFACE ${ENTROPY_INCLUDE_DIR})
else()
    add_library(ENTROPY ${ENTROPY_HEADERS})
    set_target_properties(ENTROPY PROPERTIES LINKER_LANGUAGE CXX)
    target_link_libraries(ENTROPY ${THREAD_LIB})
endif()

install(TARGETS ENTROPY DESTINATION .)
install(DIRECTORY ./include/Entropy DESTINATION include/)

