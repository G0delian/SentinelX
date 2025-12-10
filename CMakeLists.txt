cmake_minimum_required(VERSION 3.15)

project(SentinelX LANGUAGES CXX)

option(SENTINELX_USE_LIEF "Enable LIEF-based binary analysis" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(sentinelx_lib
    src/analyzer.cpp
    src/disassembler.cpp
    src/detectors.cpp
    src/fsm.cpp
    src/utils.cpp
    src/binary_parser.cpp
    src/json_output.cpp
)

target_include_directories(sentinelx_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(SENTINELX_USE_LIEF)
    message(STATUS "Using embedded LIEF")
    set(LIEF_LOGGING OFF CACHE BOOL "Disable LIEF logging")
    set(LIEF_EXAMPLES OFF CACHE BOOL "Disable LIEF examples")
    set(LIEF_TESTS OFF    CACHE BOOL "Disable LIEF tests")
    set(LIEF_PYTHON_API OFF CACHE BOOL "Disable Python API")

    add_subdirectory(LIEF)

    target_link_libraries(sentinelx_lib PUBLIC LIEF::LIEF)
    target_compile_definitions(sentinelx_lib PUBLIC SENTINELX_USE_LIEF)
endif()

add_executable(SentinelX src/main.cpp)
target_link_libraries(SentinelX PRIVATE sentinelx_lib)

if(MSVC)
    target_compile_options(sentinelx_lib PRIVATE /W4 /permissive-)
    target_compile_options(SentinelX PRIVATE /W4 /permissive-)
else()
    target_compile_options(sentinelx_lib PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(SentinelX PRIVATE -Wall -Wextra -Wpedantic)
endif()
