cmake_minimum_required(VERSION 3.3)
project(spvm)

set(CMAKE_CXX_STANDARD 11)

add_subdirectory(src)
add_subdirectory(shadertoy)

option(BUILD_EXAMPLE "Whether or not to build the example" ON)
if (${BUILD_EXAMPLE})
    message(STATUS "Building example")
    add_subdirectory(example)
endif ()

option(BUILD_TEST "Whether or not to build the tests" ON)
if (${BUILD_TEST})
    message(STATUS "Building tests")
    enable_testing()
    add_subdirectory(test)
endif ()