cmake_minimum_required(VERSION 3.16)

project(chess VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-Wall -Wextra -pedantic)
endif()

set(SRC_DIR src)
set(TST_DIR tests)

set(SOURCES
  ${SRC_DIR}/main.cpp
  ${SRC_DIR}/position.cpp
  ${SRC_DIR}/movegen.cpp
  ${SRC_DIR}/perft.cpp
  ${SRC_DIR}/move.cpp
  ${SRC_DIR}/search.cpp
  ${SRC_DIR}/engine_session.cpp
  ${TST_DIR}/perft_tests.cpp
)

add_executable(chess ${SOURCES})

# Make include/ available so <nlohmann/json.hpp> resolves
target_include_directories(chess PRIVATE ${CMAKE_SOURCE_DIR}/include)

