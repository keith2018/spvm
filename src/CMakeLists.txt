cmake_minimum_required(VERSION 3.3)
project(spvm_lib)

aux_source_directory(. SPVM_SRCS)
add_library(${PROJECT_NAME} ${SPVM_SRCS})

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR})
