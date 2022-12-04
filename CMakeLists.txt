cmake_minimum_required(VERSION 3.24)
project (mcpkg)

set(CMAKE_CXX_STANDARD 17)

add_executable(mcpkg main.cpp mcpkg/package.cpp mcpkg/package.h mcpkg/dependencies.cpp mcpkg/dependencies.h mcpkg/utils/version.cpp mcpkg/utils/version.h mcpkg/utils/result.h)
