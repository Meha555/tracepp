cmake_minimum_required(VERSION 3.16)

project(tracepp)

add_library(tracepp INTERFACE trace.hpp event.hpp)
target_include_directories(tracepp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
