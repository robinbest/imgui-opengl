#include "automation.h"
#include "picking.h"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <sstream>

void reset_recording_file(AutomationRecorder& recorder)
{
    if (recorder.path[0] == '\0')
        return;

    std::ofstream fout(recorder.path, std::ios::trunc);
    if (!fout.good())
        return;

    fout << "# recorded automation script\n";
    recorder.session_started = true;
}

void append_recorded_command(AutomationRecorder& recorder, const std::string& cmd)
{
    if (!recorder.enabled)
        return;

    if (!recorder.session_started)
        reset_recording_file(recorder);

    recorder.commands.push_back(cmd);
    recorder.last_command = cmd;

    std::ofstream fout(recorder.path, std::ios::app);
    if (fout.good())
        fout << cmd << "\n";
}

bool save_recording_snapshot(const AutomationRecorder& recorder)
{
    if (recorder.path[0] == '\0')
        return false;

    std::ofstream fout(recorder.path, std::ios::trunc);
    if (!fout.good())
        return false;

    fout << "# recorded automation script\n";
    for (const std::string& cmd : recorder.commands)
        fout << cmd << "\n";

    return true;
}

std::string run_automation_command(
    AppState& app,
    const std::string& line,
    const AutomationContext& ctx)
{
    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;

    if (cmd.empty())
        return "empty";

    if (cmd == "reset_camera")
    {
        app.camera.reset();
        return "ok reset_camera";
    }

    if (cmd == "orbit")
    {
        float dx = 0.0f, dy = 0.0f;
        iss >> dx >> dy;
        app.camera.orbit(dx, dy);
        return "ok orbit";
    }

    if (cmd == "pan")
    {
        float dx = 0.0f, dy = 0.0f;
        iss >> dx >> dy;
        app.camera.pan(dx, dy);
        return "ok pan";
    }

    if (cmd == "zoom")
    {
        float dz = 0.0f;
        iss >> dz;
        app.camera.zoom(dz);
        return "ok zoom";
    }

    if (cmd == "set_picked_face")
    {
        int face = -1;
        iss >> face;
        if (face < -1 || face > 5)
            return "error invalid_face";

        app.picked_face = face;
        return "ok set_picked_face";
    }

    if (cmd == "pick_face_at_viewport")
    {
        float x = 0.0f, y = 0.0f;
        iss >> x >> y;

        if (ctx.viewport_w <= 0.0f || ctx.viewport_h <= 0.0f)
            return "error invalid_viewport";

        if (x < 0.0f || x > ctx.viewport_w || y < 0.0f || y > ctx.viewport_h)
        {
            app.picked_face = -1;
            return "ok pick_face_at_viewport miss";
        }

        Ray ray = make_camera_ray(app.camera, x, y, ctx.viewport_w, ctx.viewport_h, ctx.fov_y_radians);
        int hit_face = -1;
        float hit_t = 0.0f;
        if (intersect_ray_unit_cube(ray, hit_face, hit_t))
        {
            app.picked_face = hit_face;
            return "ok pick_face_at_viewport";
        }

        app.picked_face = -1;
        return "ok pick_face_at_viewport miss";
    }

    if (cmd == "get_picked_face")
        return "picked_face " + std::to_string(app.picked_face);

    if (cmd == "assert_picked_face")
    {
        int expected = -1;
        iss >> expected;
        if (app.picked_face == expected)
            return "ok assert_picked_face";

        return "fail expected " + std::to_string(expected) +
               " actual " + std::to_string(app.picked_face);
    }

    return "error unknown_command";
}

bool try_load_automation_file(AutomationPlayback& playback)
{
    if (playback.active)
        return false;

    std::ifstream fin(playback.input_path);
    if (!fin.good())
        return false;

    playback.commands.clear();
    playback.next_index = 0;
    playback.wait_frames = 0;

    std::string line;
    while (std::getline(fin, line))
    {
        if (line.empty())
            continue;

        const size_t first_non_space = line.find_first_not_of(" \t\r\n");
        if (first_non_space == std::string::npos)
            continue;
        if (line[first_non_space] == '#')
            continue;

        playback.commands.push_back(line);
    }

    fin.close();

    if (playback.commands.empty())
        return false;

    playback.active = true;

    std::ofstream fout(playback.output_path, std::ios::app);
    fout << "# loaded " << playback.commands.size() << " commands\n";
    return true;
}

void process_automation_step(
    AppState& app,
    AutomationPlayback& playback,
    const AutomationContext& ctx)
{
    if (!playback.active)
        return;

    if (playback.wait_frames > 0)
    {
        playback.wait_frames--;
        return;
    }

    if (playback.next_index >= playback.commands.size())
    {
        playback.active = false;
        std::ofstream fout(playback.output_path, std::ios::app);
        fout << "# replay finished\n";
        return;
    }

    const std::string& line = playback.commands[playback.next_index++];
    if (line.empty())
        return;

    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;

    std::string result;
    if (cmd == "wait")
    {
        int frames = 0;
        iss >> frames;
        playback.wait_frames = std::max(0, frames);
        result = "ok wait";
    }
    else
    {
        result = run_automation_command(app, line, ctx);
    }

    std::ofstream fout(playback.output_path, std::ios::app);
    fout << line << " => " << result << "\n";
}
