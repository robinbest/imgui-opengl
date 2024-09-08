# 2024
project(imgui-cmake)

set(OpenGL_GL_PREFERENCE "GLVND")
find_package(OpenGL REQUIRED)

find_package(${PROJECT_NAME} QUIET COMPONENTS im-core im-glfw im-opengl3)

# ---------- Imgui Cmake Application --------

add_executable(${PROJECT_NAME} main.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${OPENGL_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME}
    ${CMAKE_DL_LIBS}
    im-core    im-glfw    im-opengl3
    gl3w ${glfw_lib} ${windows_lib}
)
