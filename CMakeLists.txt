cmake_minimum_required(VERSION 3.24)
project (mcpkg)

#add_compile_options("-pg")

SET(CMAKE_CXX_FLAGS_DEBUG -pg)

find_package(Boost COMPONENTS regex system REQUIRED)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})

    MESSAGE( STATUS "Boost_INCLUDE_DIRS = ${Boost_INCLUDE_DIRS}.")
    MESSAGE( STATUS "Boost_LIBRARIES = ${Boost_LIBRARIES}.")
    MESSAGE( STATUS "Boost_LIB_VERSION = ${Boost_LIB_VERSION}.")
endif()

set(CMAKE_CXX_STANDARD 20)

add_executable(mcpkg main.cpp mcpkg/package.cpp mcpkg/package.h mcpkg/dependencies.cpp mcpkg/dependencies.h mcpkg/utils/version.cpp mcpkg/utils/version.h mcpkg/utils/result.h)
