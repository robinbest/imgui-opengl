cmake_minimum_required(VERSION 3.15)
project(dear-imgui-conan CXX)

#find_package(imgui REQUIRED)
#find_package(glfw3 REQUIRED)
#find_package(glew REQUIRED)
find_package(${PROJECT_NAME} QUIET COMPONENTS im-core im-glfw im-opengl3)
find_package(glfw3 REQUIRED)
#find_package(glew REQUIRED)

set(backend_dir ${CMAKE_SOURCE_DIR}/submodules/imgui/backend)
add_executable( dear-imgui-conan
                main.cpp
                opengl_shader.cpp
                file_manager.cpp
                opengl_shader.h
                file_manager.h
                #${backend_dir}/bindings/imgui_impl_glfw.cpp
                #${backend_dir}/bindings/imgui_impl_glfw.h
                #${backend_dir}/bindings/imgui_impl_opengl3.cpp
                #${backend_dir}/bindings/imgui_impl_opengl3.h
                #${backend_dir}/bindings/imgui_impl_opengl3_loader.h
                assets/simple-shader.vs
                assets/simple-shader.fs )

add_custom_command(TARGET dear-imgui-conan
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/assets/simple-shader.vs $<TARGET_FILE_DIR:dear-imgui-conan>
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/assets/simple-shader.fs $<TARGET_FILE_DIR:dear-imgui-conan>
)

target_compile_definitions(dear-imgui-conan PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLEW)
#target_link_libraries(dear-imgui-conan imgui::imgui GLEW::GLEW glfw)
target_link_libraries(dear-imgui-conan
    im-core
    im-glfw
    im-opengl3
    gl3w
    GLEW
    glfw)

