set(OpenGL_GL_PREFERENCE "GLVND")
find_package(OpenGL REQUIRED)

find_package(${PROJECT_NAME} QUIET COMPONENTS im-core im-glfw im-opengl3)
find_package(glfw3 REQUIRED CONFIG)

# ---------- Imgui App --------

add_executable(${PROJECT_NAME} main.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${OPENGL_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME}
${CMAKE_DL_LIBS}
    gl3w
    im-core
    im-glfw
    im-opengl3
)
