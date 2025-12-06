cmake_minimum_required(VERSION 3.16)

project(chess
    VERSION 0.1.0
    LANGUAGES CXX
)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type default (Release if not specified)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Common warnings / optimization (like your Makefile)
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# For Release builds, enable -O3 (similar to your Makefile)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3)
endif()

# List your sources (mirrors your Makefile)
set(SRC_DIR src)
set(TST_DIR tests)

set(SOURCES
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/position.cpp
    ${SRC_DIR}/movegen.cpp
    ${SRC_DIR}/perft.cpp
    ${SRC_DIR}/move.cpp
    ${SRC_DIR}/search.cpp
    ${TST_DIR}/perft_tests.cpp
)

# Create the executable
add_executable(chess ${SOURCES})

