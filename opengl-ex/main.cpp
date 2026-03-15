#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "app_state.h"
#include "automation.h"
#include "cube_mesh.h"
#include "file_manager.h"
#include "opengl_shader.h"
#include "picking.h"
#include "ui_panels.h"
#include "viewport_renderer.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>

namespace
{
constexpr float kPi = 3.14159265358979323846f;

void glfw_error_callback(int error, const char* description)
{
    std::fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

struct RuntimeOptions
{
    bool start_record_mode = false;
    std::string start_record_path = "recorded_commands.txt";
    AutomationPlayback playback;
};

RuntimeOptions parse_runtime_options(int argc, char** argv)
{
    RuntimeOptions options;

    for (int i = 1; i < argc; ++i)
    {
        const std::string arg = argv[i];
        if (arg == "--replay" && i + 1 < argc)
        {
            options.playback.input_path = argv[++i];
            options.playback.active = false;
        }
        else if (arg == "--automation-out" && i + 1 < argc)
        {
            options.playback.output_path = argv[++i];
        }
        else if (arg == "--record")
        {
            options.start_record_mode = true;
            if (i + 1 < argc)
                options.start_record_path = argv[++i];
        }
    }

    return options;
}

void apply_scene_control_actions(
    const SceneControlsActions& actions,
    AppState& app,
    AutomationRecorder& recorder,
    AutomationPlayback& playback)
{
    if (actions.reset_camera)
    {
        app.camera.reset();
        append_recorded_command(recorder, "reset_camera");
    }

    if (actions.load_replay)
        try_load_automation_file(playback);
}

void handle_viewport_input(
    const ViewportPanelState& panel,
    AppState& app,
    AutomationRecorder& recorder,
    DragRecordingState& drag_record,
    bool& left_drag_started)
{
    if (panel.active && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
    {
        left_drag_started = true;
        const ImVec2 drag = ImGui::GetIO().MouseDelta;
        const float dx = -drag.x * 0.01f;
        const float dy = drag.y * 0.01f;
        app.camera.orbit(dx, dy);

        drag_record.orbit_active = true;
        drag_record.orbit_dx += dx;
        drag_record.orbit_dy += dy;
    }

    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
    {
        if (drag_record.orbit_active)
        {
            std::ostringstream oss;
            oss << "orbit " << drag_record.orbit_dx << " " << drag_record.orbit_dy;
            append_recorded_command(recorder, oss.str());

            drag_record.orbit_active = false;
            drag_record.orbit_dx = 0.0f;
            drag_record.orbit_dy = 0.0f;
        }
        else if (panel.hovered && !left_drag_started)
        {
            const ImVec2 mouse = ImGui::GetMousePos();
            const float local_x = mouse.x - panel.image_min.x;
            const float local_y = mouse.y - panel.image_min.y;

            if (local_x >= 0.0f && local_x <= panel.size.x &&
                local_y >= 0.0f && local_y <= panel.size.y)
            {
                Ray ray = make_camera_ray(app.camera, local_x, local_y, panel.size.x, panel.size.y, 45.0f * kPi / 180.0f);

                int hit_face = -1;
                float hit_t = 0.0f;
                if (intersect_ray_unit_cube(ray, hit_face, hit_t))
                    app.picked_face = hit_face;
                else
                    app.picked_face = -1;

                std::ostringstream action;
                action << "pick_face_at_viewport " << local_x << " " << local_y;
                append_recorded_command(recorder, action.str());

                std::ostringstream assertion;
                assertion << "assert_picked_face " << app.picked_face;
                append_recorded_command(recorder, assertion.str());
            }
        }

        left_drag_started = false;
    }

    if (panel.active && ImGui::IsMouseDragging(ImGuiMouseButton_Right))
    {
        const ImVec2 drag = ImGui::GetIO().MouseDelta;
        const float pan_speed = 0.0025f * app.camera.distance;
        const float dx = -drag.x * pan_speed;
        const float dy = drag.y * pan_speed;

        app.camera.pan(dx, dy);

        drag_record.pan_active = true;
        drag_record.pan_dx += dx;
        drag_record.pan_dy += dy;
    }

    if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
    {
        if (drag_record.pan_active)
        {
            std::ostringstream oss;
            oss << "pan " << drag_record.pan_dx << " " << drag_record.pan_dy;
            append_recorded_command(recorder, oss.str());

            drag_record.pan_active = false;
            drag_record.pan_dx = 0.0f;
            drag_record.pan_dy = 0.0f;
        }
    }

    if (panel.hovered && std::fabs(ImGui::GetIO().MouseWheel) > 0.0f)
    {
        const float dz = ImGui::GetIO().MouseWheel * 0.25f;
        app.camera.zoom(dz);

        std::ostringstream oss;
        oss << "zoom " << dz;
        append_recorded_command(recorder, oss.str());
    }
}

void clear_default_framebuffer(GLFWwindow* window)
{
    int display_w = 0;
    int display_h = 0;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.10f, 0.10f, 0.10f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

} // namespace

int main(int argc, char** argv)
{
    FileManager::init_exe_path(argv[0]);
    RuntimeOptions options = parse_runtime_options(argc, argv);

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

    if (glewInit() != GLEW_OK)
    {
        std::fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    std::cout << "GL_VERSION: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL_VERSION: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    CubeMesh cube;
    cube.initialize();

    Shader scene_shader;
    scene_shader.init(
        FileManager::read(FileManager::get_exe_path() + "/resources/simple-shader.vs"),
        FileManager::read(FileManager::get_exe_path() + "/resources/simple-shader.fs"));

    ViewportRenderer viewport_renderer;
    viewport_renderer.initialize(&scene_shader);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    ImGui::StyleColorsDark();

    AppState app;
    AutomationRecorder recorder;
    DragRecordingState drag_record;
    bool left_drag_started = false;

    if (options.start_record_mode)
    {
        recorder.enabled = true;
        std::snprintf(recorder.path, sizeof(recorder.path), "%s", options.start_record_path.c_str());
        reset_recording_file(recorder);
    }

    const char* face_names[6] = {
        "face 1 (front)",
        "face 2 (back)",
        "face 3 (left)",
        "face 4 (right)",
        "face 5 (top)",
        "face 6 (bottom)"
    };

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        const SceneControlsActions control_actions =
            draw_scene_controls_panel(app, recorder, options.playback, face_names);
        apply_scene_control_actions(control_actions, app, recorder, options.playback);

        viewport_renderer.resize(viewport_renderer.width(), viewport_renderer.height());
        const ViewportPanelState viewport_panel = draw_viewport_panel(viewport_renderer.texture_id());

        if ((int)viewport_panel.size.x != viewport_renderer.width() ||
            (int)viewport_panel.size.y != viewport_renderer.height())
        {
            viewport_renderer.resize((int)viewport_panel.size.x, (int)viewport_panel.size.y);
        }

        handle_viewport_input(viewport_panel, app, recorder, drag_record, left_drag_started);

        viewport_renderer.render(app, cube);

        clear_default_framebuffer(window);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);

        if (!options.playback.active && options.playback.rewind)
            try_load_automation_file(options.playback);

        AutomationContext automation_ctx;
        automation_ctx.viewport_w = static_cast<float>(viewport_renderer.width());
        automation_ctx.viewport_h = static_cast<float>(viewport_renderer.height());
        automation_ctx.fov_y_radians = 45.0f * kPi / 180.0f;

        process_automation_step(app, options.playback, automation_ctx);
    }

    if (recorder.enabled)
        save_recording_snapshot(recorder);

    viewport_renderer.shutdown();
    cube.shutdown();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
