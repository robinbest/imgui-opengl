#include "imgui.h"
//#include "bindings/imgui_impl_glfw.h"
//#include "bindings/imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "opengl_shader.h"
#include "file_manager.h"

#include <stdio.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdint>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define PI 3.14159265358979323846f

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

struct Mat4
{
    float v[16];
};

static Mat4 identity()
{
    Mat4 m = {};
    m.v[0] = 1.0f;
    m.v[5] = 1.0f;
    m.v[10] = 1.0f;
    m.v[15] = 1.0f;
    return m;
}

static Mat4 multiply(const Mat4& a, const Mat4& b)
{
    Mat4 out = {};
    for (int col = 0; col < 4; ++col)
    {
        for (int row = 0; row < 4; ++row)
        {
            out.v[col * 4 + row] =
                a.v[0 * 4 + row] * b.v[col * 4 + 0] +
                a.v[1 * 4 + row] * b.v[col * 4 + 1] +
                a.v[2 * 4 + row] * b.v[col * 4 + 2] +
                a.v[3 * 4 + row] * b.v[col * 4 + 3];
        }
    }
    return out;
}

static Mat4 perspective(float fov_y_radians, float aspect, float z_near, float z_far)
{
    Mat4 m = {};
    const float t = std::tan(fov_y_radians * 0.5f);

    m.v[0] = 1.0f / (aspect * t);
    m.v[5] = 1.0f / t;
    m.v[10] = -(z_far + z_near) / (z_far - z_near);
    m.v[11] = -1.0f;
    m.v[14] = -(2.0f * z_far * z_near) / (z_far - z_near);

    return m;
}

static Mat4 translate(float x, float y, float z)
{
    Mat4 m = identity();
    m.v[12] = x;
    m.v[13] = y;
    m.v[14] = z;
    return m;
}

static Mat4 rotate_x(float angle)
{
    Mat4 m = identity();
    const float c = std::cos(angle);
    const float s = std::sin(angle);

    m.v[5] = c;
    m.v[6] = s;
    m.v[9] = -s;
    m.v[10] = c;

    return m;
}

static Mat4 rotate_y(float angle)
{
    Mat4 m = identity();
    const float c = std::cos(angle);
    const float s = std::sin(angle);

    m.v[0] = c;
    m.v[2] = -s;
    m.v[8] = s;
    m.v[10] = c;

    return m;
}

static void create_cube(unsigned int& vbo, unsigned int& vao)
{
    const float cube_vertices[] = {
        // positions            // colors

        // front
        -0.5f, -0.5f,  0.5f,    1.0f, 0.2f, 0.2f,
         0.5f, -0.5f,  0.5f,    1.0f, 0.2f, 0.2f,
         0.5f,  0.5f,  0.5f,    1.0f, 0.2f, 0.2f,
         0.5f,  0.5f,  0.5f,    1.0f, 0.2f, 0.2f,
        -0.5f,  0.5f,  0.5f,    1.0f, 0.2f, 0.2f,
        -0.5f, -0.5f,  0.5f,    1.0f, 0.2f, 0.2f,

        // back
        -0.5f, -0.5f, -0.5f,    0.2f, 1.0f, 0.2f,
        -0.5f,  0.5f, -0.5f,    0.2f, 1.0f, 0.2f,
         0.5f,  0.5f, -0.5f,    0.2f, 1.0f, 0.2f,
         0.5f,  0.5f, -0.5f,    0.2f, 1.0f, 0.2f,
         0.5f, -0.5f, -0.5f,    0.2f, 1.0f, 0.2f,
        -0.5f, -0.5f, -0.5f,    0.2f, 1.0f, 0.2f,

        // left
        -0.5f, -0.5f, -0.5f,    0.2f, 0.4f, 1.0f,
        -0.5f, -0.5f,  0.5f,    0.2f, 0.4f, 1.0f,
        -0.5f,  0.5f,  0.5f,    0.2f, 0.4f, 1.0f,
        -0.5f,  0.5f,  0.5f,    0.2f, 0.4f, 1.0f,
        -0.5f,  0.5f, -0.5f,    0.2f, 0.4f, 1.0f,
        -0.5f, -0.5f, -0.5f,    0.2f, 0.4f, 1.0f,

        // right
         0.5f, -0.5f, -0.5f,    1.0f, 0.8f, 0.2f,
         0.5f,  0.5f, -0.5f,    1.0f, 0.8f, 0.2f,
         0.5f,  0.5f,  0.5f,    1.0f, 0.8f, 0.2f,
         0.5f,  0.5f,  0.5f,    1.0f, 0.8f, 0.2f,
         0.5f, -0.5f,  0.5f,    1.0f, 0.8f, 0.2f,
         0.5f, -0.5f, -0.5f,    1.0f, 0.8f, 0.2f,

         // top
         -0.5f,  0.5f, -0.5f,    0.7f, 0.2f, 1.0f,
         -0.5f,  0.5f,  0.5f,    0.7f, 0.2f, 1.0f,
          0.5f,  0.5f,  0.5f,    0.7f, 0.2f, 1.0f,
          0.5f,  0.5f,  0.5f,    0.7f, 0.2f, 1.0f,
          0.5f,  0.5f, -0.5f,    0.7f, 0.2f, 1.0f,
         -0.5f,  0.5f, -0.5f,    0.7f, 0.2f, 1.0f,

         // bottom
         -0.5f, -0.5f, -0.5f,    0.2f, 1.0f, 1.0f,
          0.5f, -0.5f, -0.5f,    0.2f, 1.0f, 1.0f,
          0.5f, -0.5f,  0.5f,    0.2f, 1.0f, 1.0f,
          0.5f, -0.5f,  0.5f,    0.2f, 1.0f, 1.0f,
         -0.5f, -0.5f,  0.5f,    0.2f, 1.0f, 1.0f,
         -0.5f, -0.5f, -0.5f,    0.2f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

static void create_viewport_fbo(GLuint& fbo, GLuint& color_tex, GLuint& depth_rbo, int width, int height)
{
    if (fbo != 0)
    {
        glDeleteFramebuffers(1, &fbo);
        glDeleteTextures(1, &color_tex);
        glDeleteRenderbuffers(1, &depth_rbo);
        fbo = 0;
        color_tex = 0;
        depth_rbo = 0;
    }

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &color_tex);
    glBindTexture(GL_TEXTURE_2D, color_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_tex, 0);

    glGenRenderbuffers(1, &depth_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "ERROR: viewport framebuffer is not complete." << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int main(int narg, char** argv)
{
    FileManager::init_exe_path(argv[0]);

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

#if __APPLE__
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
    const char* glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui - OpenGL Viewport", NULL, NULL);
    if (window == NULL)
        return 1;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    bool err = glewInit() != GLEW_OK;
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    std::cout << "GL_VERSION: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL_VERSION: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    unsigned int vbo = 0;
    unsigned int vao = 0;
    create_cube(vbo, vao);

    Shader scene_shader;
    scene_shader.init(
        FileManager::read(FileManager::get_exe_path() + "/resources/simple-shader.vs"),
        FileManager::read(FileManager::get_exe_path() + "/resources/simple-shader.fs"));

    GLuint viewport_fbo = 0;
    GLuint viewport_tex = 0;
    GLuint viewport_depth = 0;
    int viewport_fb_w = 1;
    int viewport_fb_h = 1;
    create_viewport_fbo(viewport_fbo, viewport_tex, viewport_depth, viewport_fb_w, viewport_fb_h);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    ImGui::StyleColorsDark();

    float yaw = 0.6f;
    float pitch = 0.4f;
    float distance = 3.0f;
    float target_x = 0.0f;
    float target_y = 0.0f;
    float target_z = 0.0f;
    float tint[3] = { 1.0f, 1.0f, 1.0f };

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Scene Controls");
        ImGui::Text("Direct mouse controls in the viewport:");
        ImGui::BulletText("Left drag: orbit");
        ImGui::BulletText("Right drag: pan");
        ImGui::BulletText("Mouse wheel: zoom");
        ImGui::ColorEdit3("Tint", tint);

        if (ImGui::Button("Reset View"))
        {
            yaw = 0.6f;
            pitch = 0.4f;
            distance = 3.0f;
            target_x = 0.0f;
            target_y = 0.0f;
        }
        ImGui::End();

        ImGui::Begin("3D Render View");

        ImVec2 avail = ImGui::GetContentRegionAvail();
        if (avail.x < 50.0f) avail.x = 50.0f;
        if (avail.y < 50.0f) avail.y = 50.0f;

        int desired_w = static_cast<int>(avail.x);
        int desired_h = static_cast<int>(avail.y);

        if (desired_w != viewport_fb_w || desired_h != viewport_fb_h)
        {
            viewport_fb_w = desired_w;
            viewport_fb_h = desired_h;
            create_viewport_fbo(viewport_fbo, viewport_tex, viewport_depth, viewport_fb_w, viewport_fb_h);
        }

        // Create an interactive region for the viewport.
        ImGui::InvisibleButton(
            "viewport_canvas",
            avail,
            ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);

        const bool viewport_hovered = ImGui::IsItemHovered();
        const bool viewport_active = ImGui::IsItemActive();
        const ImVec2 image_min = ImGui::GetItemRectMin();
        const ImVec2 image_max = ImGui::GetItemRectMax();

        // Draw the rendered texture into the exact same rect.
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        draw_list->AddImage(
            (ImTextureID)(intptr_t)viewport_tex,
            image_min,
            image_max,
            ImVec2(0, 1),
            ImVec2(1, 0));

        // Optional border
        draw_list->AddRect(image_min, image_max, IM_COL32(255, 255, 255, 40));

        if (viewport_active && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {
            const ImVec2 drag = ImGui::GetIO().MouseDelta;
            //yaw += drag.x * 0.01f;
            //pitch += drag.y * 0.01f;
            yaw -= drag.x * 0.01f;
            pitch -= drag.y * 0.01f;

            const float pitch_limit = 1.45f;
            pitch = std::clamp(pitch, -pitch_limit, pitch_limit);
        }

        if (viewport_active && ImGui::IsMouseDragging(ImGuiMouseButton_Right))
        {
            const ImVec2 drag = ImGui::GetIO().MouseDelta;
            const float pan_speed = 0.0025f * distance;

            // Camera right vector projected onto the ground plane.
            const float cy = std::cos(yaw);
            const float sy = std::sin(yaw);

            const float right_x = cy;
            const float right_y = 0.0f;
            const float right_z = -sy;

            // Simple camera up vector.
            const float up_x = 0.0f;
            const float up_y = 1.0f;
            const float up_z = 0.0f;

            const float dx = -drag.x * pan_speed;
            const float dy = drag.y * pan_speed;

            target_x += right_x * dx + up_x * dy;
            target_y += right_y * dx + up_y * dy;
            target_z += right_z * dx + up_z * dy;
        }

        if (viewport_hovered && std::fabs(ImGui::GetIO().MouseWheel) > 0.0f)
        {
            distance -= ImGui::GetIO().MouseWheel * 0.25f;
            distance = std::clamp(distance, 1.5f, 10.0f);
        }

        ImGui::End();

        glBindFramebuffer(GL_FRAMEBUFFER, viewport_fbo);
        glViewport(0, 0, viewport_fb_w, viewport_fb_h);
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.12f, 0.13f, 0.16f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const float aspect = static_cast<float>(viewport_fb_w) / static_cast<float>(viewport_fb_h);
        Mat4 proj = perspective(45.0f * PI / 180.0f, aspect, 0.1f, 100.0f);

        Mat4 model = identity();

        // Pan by shifting the scene in camera space before the orbit rotation.
        Mat4 target_translate = translate(-target_x, -target_y, -target_z);
        Mat4 orbit = multiply(rotate_x(-pitch), rotate_y(-yaw));
        Mat4 dolly = translate(0.0f, 0.0f, -distance);

        Mat4 view = multiply(dolly, multiply(orbit, target_translate));
        Mat4 mvp = multiply(proj, multiply(view, model));

        scene_shader.use();
        scene_shader.setUniformMat4("u_mvp", mvp.v);
        scene_shader.setUniform("tint", tint[0], tint[1], tint[2]);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        int display_w = 0;
        int display_h = 0;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.10f, 0.10f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    glDeleteFramebuffers(1, &viewport_fbo);
    glDeleteTextures(1, &viewport_tex);
    glDeleteRenderbuffers(1, &viewport_depth);

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
