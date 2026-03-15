#pragma once

#include "app_state.h"

#include <string>
#include <vector>

struct AutomationContext
{
    float viewport_w = 0.0f;
    float viewport_h = 0.0f;
    float fov_y_radians = 0.0f;
};

struct AutomationRecorder
{
    bool enabled = false;
    bool session_started = false;
    char path[256] = "recorded_commands.txt";
    std::vector<std::string> commands;
    std::string last_command;
};

struct DragRecordingState
{
    bool orbit_active = false;
    bool pan_active = false;
    float orbit_dx = 0.0f;
    float orbit_dy = 0.0f;
    float pan_dx = 0.0f;
    float pan_dy = 0.0f;
};

struct AutomationPlayback
{
    bool active = false;
    bool rewind = false;
    std::vector<std::string> commands;
    size_t next_index = 0;
    int wait_frames = 0;
    std::string input_path = "automation_in.txt";
    std::string output_path = "automation_out.txt";
};

void reset_recording_file(AutomationRecorder& recorder);
void append_recorded_command(AutomationRecorder& recorder, const std::string& cmd);
bool save_recording_snapshot(const AutomationRecorder& recorder);

std::string run_automation_command(
    AppState& app,
    const std::string& line,
    const AutomationContext& ctx);

bool try_load_automation_file(AutomationPlayback& playback);
void process_automation_step(
    AppState& app,
    AutomationPlayback& playback,
    const AutomationContext& ctx);
