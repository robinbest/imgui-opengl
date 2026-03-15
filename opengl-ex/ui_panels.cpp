#include "ui_panels.h"

SceneControlsActions draw_scene_controls_panel(
    AppState& app,
    AutomationRecorder& recorder,
    AutomationPlayback& playback,
    const char* const* face_names)
{
    SceneControlsActions actions{};

    ImGui::Begin("Scene Controls");
    ImGui::Text("Direct mouse controls in the viewport:");
    ImGui::BulletText("Left drag: orbit");
    ImGui::BulletText("Right drag: pan");
    ImGui::BulletText("Mouse wheel: zoom");
    ImGui::ColorEdit3("Tint", app.tint);

    if (app.picked_face >= 0)
        ImGui::Text("%s is picked", face_names[app.picked_face]);
    else
        ImGui::Text("No face is picked");

    if (ImGui::Button("Reset View"))
        actions.reset_camera = true;

    ImGui::Separator();
    bool record_enabled = recorder.enabled;
    if (ImGui::Checkbox("Record automation", &record_enabled))
    {
        recorder.enabled = record_enabled;
        if (recorder.enabled)
            reset_recording_file(recorder);
    }

    ImGui::InputText("Record file", recorder.path, IM_ARRAYSIZE(recorder.path));

    if (ImGui::Button("Clear Recording"))
    {
        recorder.commands.clear();
        recorder.last_command.clear();
        if (recorder.enabled)
            reset_recording_file(recorder);
    }

    ImGui::SameLine();

    if (ImGui::Button("Save Recording"))
        save_recording_snapshot(recorder);

    ImGui::Text("Recorded commands: %d", static_cast<int>(recorder.commands.size()));
    if (!recorder.last_command.empty())
        ImGui::Text("Last recorded: %s", recorder.last_command.c_str());

    ImGui::Separator();
    ImGui::Text("Replay input: %s", playback.input_path.c_str());
    ImGui::Text("Replay output: %s", playback.output_path.c_str());

    if (playback.active)
        ImGui::Text("Replay active: step %d / %d", (int)playback.next_index, (int)playback.commands.size());
    else
        ImGui::Text("Replay idle");

    if (ImGui::Button("Load Replay Script"))
        actions.load_replay = true;

    ImGui::End();
    return actions;
}

ViewportPanelState draw_viewport_panel(GLuint texture_id)
{
    ViewportPanelState state{};

    ImGui::Begin("3D Render View");

    state.size = ImGui::GetContentRegionAvail();
    if (state.size.x < 50.0f) state.size.x = 50.0f;
    if (state.size.y < 50.0f) state.size.y = 50.0f;

    ImGui::InvisibleButton(
        "viewport_canvas",
        state.size,
        ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);

    state.hovered = ImGui::IsItemHovered();
    state.active = ImGui::IsItemActive();
    state.image_min = ImGui::GetItemRectMin();
    state.image_max = ImGui::GetItemRectMax();

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddImage(
        (ImTextureID)(intptr_t)texture_id,
        state.image_min,
        state.image_max,
        ImVec2(0, 1),
        ImVec2(1, 0));

    draw_list->AddRect(state.image_min, state.image_max, IM_COL32(255, 255, 255, 40));

    ImGui::End();
    return state;
}
