# 2024

cmake_minimum_required(VERSION 3.15)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(project imgui-study)
project(${project} VERSION 1.0)

configure_file(cmake-ex/include/config.h.in cmake-ex/include/config.h)

#add_subdirectory(submodules/gl3w)

set(GL3W_DIR ${CMAKE_BINARY_DIR}/submodules/gl3w)
set(HEADER_DIR ${GL3W_DIR}/include)

#build imgui libs
add_subdirectory(submodules)

#original cmake example
include_directories(${GL3W_DIR}/include)
add_subdirectory(cmake-ex)

#opengl example
include_directories(submodules/imgui/backends)
add_subdirectory(opengl-ex)
