# 2024

cmake_minimum_required(VERSION 3.10)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(project imgui-opengl)
project(${project} VERSION 1.0)

configure_file(src/include/config.h.in src/include/config.h)

#add_subdirectory(submodules/gl3w)

include_directories(submodules/gl3w/include)

set(IMGUI_PATH ${CMAKE_SOURCE_DIR}/src/imgui)
add_subdirectory(submodules)

#cmake example
add_subdirectory(cmake-ex)

#opengl example
include_directories(submodules/imgui/backends)
add_subdirectory(opengl-ex)

