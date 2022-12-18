cmake_minimum_required(VERSION 3.24)
project (mcpkg)

#SET(CMAKE_CXX_FLAGS_DEBUG -pg)

set(CMAKE_CXX_STANDARD 20)

# Packages
MESSAGE(STATUS "_VCPKG_INSTALLED_DIR = ${_VCPKG_INSTALLED_DIR}/")
list(APPEND CMAKE_PREFIX_PATH "${_VCPKG_INSTALLED_DIR}/x64-linux/share/") # add vcpkg packages

# BOOST
set(Boost_INCLUDE_DIR ${_VCPKG_INSTALLED_DIR}/x64-linux/include)
find_package(Boost COMPONENTS regex system REQUIRED)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})

    MESSAGE( STATUS "Boost_VERSION = ${Boost_VERSION}.")
    MESSAGE( STATUS "Boost_INCLUDE_DIRS = ${Boost_INCLUDE_DIRS}.")
    MESSAGE( STATUS "Boost_LIBRARIES = ${Boost_LIBRARIES}.")
endif()


# SQLITE
set(tomlplusplus_DIR ${_VCPKG_INSTALLED_DIR}/x64-linux/)
find_package(SQLite3 REQUIRED)
if (SQLite3_FOUND)
    include_directories(${SQLite3_INCLUDE_DIRS}/../external/sqlite3/include)
    link_libraries(${SQLite3_LIBRARIES})

    MESSAGE( STATUS "SQLite3_VERSION = ${SQLite3_VERSION}.")
    MESSAGE( STATUS "SQLite3_INCLUDE_DIRS = ${SQLite3_INCLUDE_DIRS}.")
    MESSAGE( STATUS "SQLite3_LIBRARIES = ${SQLite3_LIBRARIES}.")
endif (SQLite3_FOUND)


# TOML++
find_package(tomlplusplus CONFIG REQUIRED)
if (tomlplusplus_FOUND)
    MESSAGE( STATUS "tomlplusplus_VERSION = ${tomlplusplus_VERSION}.")
endif (tomlplusplus_FOUND)

#file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/pkgs DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(mcpkg main.cpp mcpkg/package.cpp mcpkg/package.h mcpkg/dependencies.cpp mcpkg/dependencies.h mcpkg/utils/version.cpp mcpkg/utils/version.h mcpkg/utils/result.h mcpkg/utils/database.cpp mcpkg/utils/database.h)
