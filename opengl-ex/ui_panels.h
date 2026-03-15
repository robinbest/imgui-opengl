#pragma once

#include "app_state.h"
#include "automation.h"

#include "imgui.h"

#include <GL/glew.h>

struct SceneControlsActions
{
    bool reset_camera = false;
    bool load_replay = false;
};

struct ViewportPanelState
{
    ImVec2 size = ImVec2(50.0f, 50.0f);
    ImVec2 image_min = ImVec2(0.0f, 0.0f);
    ImVec2 image_max = ImVec2(0.0f, 0.0f);
    bool hovered = false;
    bool active = false;
};

SceneControlsActions draw_scene_controls_panel(
    AppState& app,
    AutomationRecorder& recorder,
    AutomationPlayback& playback,
    const char* const* face_names);

ViewportPanelState draw_viewport_panel(GLuint texture_id);
