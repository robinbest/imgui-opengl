# 2024

project(imgui-opengl CXX)

find_package(glfw3 REQUIRED)
#find_package(glew REQUIRED)

set(backend_dir ${CMAKE_SOURCE_DIR}/submodules/imgui/backend)
add_executable( ${PROJECT_NAME}
                main.cpp
                opengl_shader.cpp
                file_manager.cpp
                opengl_shader.h
                file_manager.h
                assets/simple-shader.vs
                assets/simple-shader.fs )

add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/assets/simple-shader.vs $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/assets/simple-shader.fs $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

target_compile_definitions(${PROJECT_NAME} PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLEW)
target_link_libraries(${PROJECT_NAME}
    im-core
    im-glfw
    im-opengl3
    gl3w
    GLEW
    glfw)

