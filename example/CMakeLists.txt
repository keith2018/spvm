cmake_minimum_required(VERSION 3.3)
project(spvm_example)

set(CMAKE_CXX_STANDARD 11)

include_directories("../src")
add_executable(${PROJECT_NAME} main.cpp)

set(LIBRARIES spvm_lib)
target_link_libraries(${PROJECT_NAME} ${LIBRARIES})

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif ()

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/../bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/../bin)

# copy assets
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove_directory $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
        )