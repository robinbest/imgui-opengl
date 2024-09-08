# 2024

project(imgui-opengl CXX)

set(OpenGL_GL_PREFERENCE "GLVND")
find_package(OpenGL REQUIRED)

set(backend_dir ${CMAKE_SOURCE_DIR}/submodules/imgui/backend)
add_executable( ${PROJECT_NAME}
                main.cpp
                opengl_shader.cpp
                file_manager.cpp
                opengl_shader.h
                file_manager.h
                assets/simple-shader.vs
                assets/simple-shader.fs )

set(resources_dest $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources)
add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${resources_dest}
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/assets/simple-shader.vs ${resources_dest}
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/assets/simple-shader.fs ${resources_dest}
)
if (WIN32)
    add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${glew_DIR}/bin/Release/x64/glew32.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif ()

target_compile_definitions(${PROJECT_NAME} PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLEW)
target_link_libraries(${PROJECT_NAME}
    im-core im-glfw im-opengl3
    gl3w ${glew_lib} ${glfw_lib} ${windows_lib}
)

