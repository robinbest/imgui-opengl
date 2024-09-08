# 2024

# generate gl3w files
# this is a separate step to generate files so that gl3w.c can be included for downstream targets
include(gl3w.cmake)
# build
add_subdirectory(gl3w)

if (WIN32)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "dont build glfw examples")
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "dont build glfw tests")
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "dont build glfw docs")
    add_subdirectory(glfw)
endif ()

#------------------------------------------------------
set(IMGUI_PATH ${CMAKE_SOURCE_DIR}/submodules/imgui)

find_package(OpenGL REQUIRED)
set(OpenGL_GL_PREFERENCE "GLVND")

# ------ Core -------
add_library(im-core 
    ${IMGUI_PATH}/imgui.cpp
    ${IMGUI_PATH}/imgui_draw.cpp
    ${IMGUI_PATH}/imgui_widgets.cpp
    ${IMGUI_PATH}/imgui_tables.cpp
)
target_include_directories(im-core 
    PUBLIC
    $<BUILD_INTERFACE:${IMGUI_PATH}>
    $<INSTALL_INTERFACE:imgui.h>
    $<INSTALL_INTERFACE:imgui_internal.h>
)
# namespaced alias
add_library(${project}::im-core ALIAS im-core)

# ------ GLFW -------
add_library(im-glfw ${IMGUI_PATH}/backends/imgui_impl_glfw.cpp)
target_include_directories(im-glfw
    PUBLIC
    $<BUILD_INTERFACE:${IMGUI_PATH}/backends>
    $<INSTALL_INTERFACE:imgui_impl_glfw.h>
)
target_link_libraries(im-glfw
    ${OPENGL_LIBRARIES}
    im-core
    glfw)
add_library(${project}::im-glfw ALIAS im-glfw)


# ------ OpenGL3 -------
set(GLLOADER_LIBRARIES gl3w)
add_library(im-opengl3 ${IMGUI_PATH}/backends/imgui_impl_opengl3.cpp)
target_include_directories(im-opengl3
    PUBLIC
    $<BUILD_INTERFACE:${IMGUI_PATH}/backends >
    $<INSTALL_INTERFACE:imgui_impl_opengl3.h>
    ${GLLOADER_INCLUDES}
    PRIVATE ${IMGUI_PATH}
)

target_link_libraries(im-opengl3
    ${GLLOADER_LIBRARIES}
    OpenGL::GL
)
if (NOT WIN32)
    target_link_libraries(im-opengl3  OpenGL::GLX)
endif()

add_library(${project}::im-opengl3 ALIAS im-opengl3)