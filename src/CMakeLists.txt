cmake_minimum_required(VERSION 3.3)
project(spvm_lib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(-DDEBUG)
endif ()

aux_source_directory(. SPVM_SRCS)
add_library(${PROJECT_NAME} ${SPVM_SRCS})

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR})
