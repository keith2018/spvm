cmake_minimum_required(VERSION 3.3)
project(spvm_shadertoy)

set(CMAKE_CXX_STANDARD 11)
set(THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party")

include_directories(
        "${THIRD_PARTY_DIR}/glfw/include"
        "${THIRD_PARTY_DIR}/glad/include"
        "${THIRD_PARTY_DIR}/imgui"
        "${THIRD_PARTY_DIR}/stb/include"
        "${THIRD_PARTY_DIR}/glslang/include"
        "../src"
)

set(SKIP_GLSLANG_INSTALL ON)
set(ENABLE_GLSLANG_BINARIES OFF)
set(ENABLE_CTEST OFF)
set(BUILD_TESTING OFF)
add_subdirectory("${THIRD_PARTY_DIR}/glslang")

# imgui src
file(GLOB IMGUI_SRC
        ${THIRD_PARTY_DIR}/imgui/imgui/*.cpp
        )

add_executable(${PROJECT_NAME}
        "${IMGUI_SRC}"
        "${THIRD_PARTY_DIR}/glad/src/glad.c"
        renderer.cpp
        compiler.cpp
        resourcelimits.cpp
        timer.cpp
        main.cpp
        )

set(LIBRARIES
        spvm_lib
        glslang
        SPIRV)

if (WIN32)
    if (MSVC)
        target_link_libraries(${PROJECT_NAME}
                ${LIBRARIES}
                "${THIRD_PARTY_DIR}/glfw/lib-vc2022/glfw3.lib"
                "${THIRD_PARTY_DIR}/glfw/lib-vc2022/glfw3dll.lib"
                )
    else ()
        target_link_libraries(${PROJECT_NAME}
                ${LIBRARIES}
                "${THIRD_PARTY_DIR}/glfw/lib-mingw-w64/libglfw3.a"
                "${THIRD_PARTY_DIR}/glfw/lib-mingw-w64/libglfw3dll.a"
                )
    endif ()
endif ()

if (APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -framework Cocoa -framework OpenGL -framework IOKit")
    add_compile_definitions(GL_SILENCE_DEPRECATION)
    target_link_libraries(${PROJECT_NAME}
            ${LIBRARIES}
            "${THIRD_PARTY_DIR}/glfw/lib-macos-universal/libglfw3.a"
            )
endif ()

if (UNIX AND NOT APPLE)
    find_package(OpenGL REQUIRED)
    target_link_libraries(${PROJECT_NAME}
            ${LIBRARIES}
            glfw
            OpenGL::GL
            pthread
            ${CMAKE_DL_LIBS}
            )
endif ()

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif ()

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/../bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/../bin)

# copy assets
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove_directory $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
        )