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
#include <sstream>
#include <fstream>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define PI 3.14159265358979323846f

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

struct Vec3
{
    float x, y, z;
};

static Vec3 make_vec3(float x, float y, float z)
{
    return { x, y, z };
}

static Vec3 add(const Vec3& a, const Vec3& b)
{
    return { a.x + b.x, a.y + b.y, a.z + b.z };
}

static Vec3 sub(const Vec3& a, const Vec3& b)
{
    return { a.x - b.x, a.y - b.y, a.z - b.z };
}

static Vec3 mul(const Vec3& v, float s)
{
    return { v.x * s, v.y * s, v.z * s };
}

static float dot(const Vec3& a, const Vec3& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static Vec3 cross(const Vec3& a, const Vec3& b)
{
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

static float length(const Vec3& v)
{
    return std::sqrt(dot(v, v));
}

static Vec3 normalize(const Vec3& v)
{
    float len = length(v);
    if (len < 1e-6f)
        return { 0.0f, 0.0f, 0.0f };
    return { v.x / len, v.y / len, v.z / len };
}

static Vec3 negate(const Vec3& v)
{
    return { -v.x, -v.y, -v.z };
}

//---------------------------------
struct Vec4
{
    float x, y, z, w;
};

//---------------------------------
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

static Mat4 inverse_rigid_body(const Mat4& m)
{
    // Assumes matrix = rotation + translation, no scale/shear.
    Mat4 out = identity();

    // Transpose the upper 3x3 rotation part.
    out.v[0] = m.v[0];
    out.v[1] = m.v[4];
    out.v[2] = m.v[8];

    out.v[4] = m.v[1];
    out.v[5] = m.v[5];
    out.v[6] = m.v[9];

    out.v[8] = m.v[2];
    out.v[9] = m.v[6];
    out.v[10] = m.v[10];

    Vec3 t = { m.v[12], m.v[13], m.v[14] };

    out.v[12] = -(out.v[0] * t.x + out.v[4] * t.y + out.v[8] * t.z);
    out.v[13] = -(out.v[1] * t.x + out.v[5] * t.y + out.v[9] * t.z);
    out.v[14] = -(out.v[2] * t.x + out.v[6] * t.y + out.v[10] * t.z);

    return out;
}

static Vec3 transform_point(const Mat4& m, const Vec3& p)
{
    return {
        m.v[0] * p.x + m.v[4] * p.y + m.v[8] * p.z + m.v[12],
        m.v[1] * p.x + m.v[5] * p.y + m.v[9] * p.z + m.v[13],
        m.v[2] * p.x + m.v[6] * p.y + m.v[10] * p.z + m.v[14]
    };
}

static Vec3 transform_vector(const Mat4& m, const Vec3& v)
{
    return {
        m.v[0] * v.x + m.v[4] * v.y + m.v[8] * v.z,
        m.v[1] * v.x + m.v[5] * v.y + m.v[9] * v.z,
        m.v[2] * v.x + m.v[6] * v.y + m.v[10] * v.z
    };
}

//real camera view transform instead of building view from ad hoc matrix composition
static Mat4 look_at(const Vec3& eye, const Vec3& target, const Vec3& up)
{
    /*
      eye    = camera_position
      center = target
      up     = camera up direction
      OpenGL: Place the camera at eye and rotate it so it looks at center.
    */
    Vec3 f = normalize(sub(target, eye));
    Vec3 s = normalize(cross(f, up));
    Vec3 u = cross(s, f);

    Mat4 m = identity();

    m.v[0] = s.x;
    m.v[1] = u.x;
    m.v[2] = -f.x;
    m.v[3] = 0.0f;

    m.v[4] = s.y;
    m.v[5] = u.y;
    m.v[6] = -f.y;
    m.v[7] = 0.0f;

    m.v[8] = s.z;
    m.v[9] = u.z;
    m.v[10] = -f.z;
    m.v[11] = 0.0f;

    m.v[12] = -dot(s, eye);
    m.v[13] = -dot(u, eye);
    m.v[14] = dot(f, eye);
    m.v[15] = 1.0f;

    return m;
}

/**
  Orbit cameras are ideal for editors because:
    rotation happens around the object
    zoom moves toward the object
    pan moves the object under the cursor

    Control    What changes
      rotate    camera position
      zoom      camera distance
      pan       target
*/
struct OrbitCamera
{
    Vec3 target = { 0.0f, 0.0f, 0.0f };
    float yaw = 0.6f;
    float pitch = 0.4f;
    float distance = 3.0f;

    Vec3 position() const
    {
        const float cp = std::cos(pitch);
        const float sp = std::sin(pitch);
        const float cy = std::cos(yaw);
        const float sy = std::sin(yaw);

        // Spherical orbit around target
        Vec3 offset = {
            distance * cp * sy,
            distance * sp,
            distance * cp * cy
        };

        return add(target, offset);
    }

    Vec3 forward() const
    {
        return normalize(sub(target, position()));
    }

    Vec3 right() const
    {
        const Vec3 world_up = { 0.0f, 1.0f, 0.0f };
        return normalize(cross(forward(), world_up));
    }

    Vec3 up() const
    {
        return normalize(cross(right(), forward()));
    }

    Mat4 view_matrix() const
    {
        return look_at(position(), target, up());
    }

    void orbit(float dx, float dy)
    {
        yaw += dx;
        pitch += dy;

        const float pitch_limit = 1.45f;
        pitch = std::clamp(pitch, -pitch_limit, pitch_limit);
    }

    void zoom(float delta)
    {
        distance -= delta;
        distance = std::clamp(distance, 1.5f, 20.0f);
    }

    void pan(float dx, float dy)
    {
        Vec3 r = right();
        Vec3 u = up();

        target = add(target, add(mul(r, dx), mul(u, dy)));
    }

    Vec3 world_up() const
    {
        return { 0.0f, 1.0f, 0.0f };
    }

    Vec3 basis_right() const
    {
        return right();
    }

    Vec3 basis_up() const
    {
        return up();
    }

    void reset()
    {
        target = { 0.0f, 0.0f, 0.0f };
        yaw = 0.6f;
        pitch = 0.4f;
        distance = 3.0f;
    }
};

struct Ray
{
    Vec3 origin;
    Vec3 dir;
};

static Ray make_camera_ray(
    const OrbitCamera& camera,
    float mouse_x,
    float mouse_y,
    float viewport_w,
    float viewport_h,
    float fov_y_radians);
static bool intersect_ray_unit_cube(const Ray& ray, int& out_face, float& out_t);

//-------------------------------
struct AppState
{
    OrbitCamera camera;
    int picked_face = -1;
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

static void reset_recording_file(AutomationRecorder& recorder)
{
    if (recorder.path[0] == '\0')
        return;

    std::ofstream fout(recorder.path, std::ios::trunc);
    if (!fout.good())
        return;

    fout << "# recorded automation script\n";
    recorder.session_started = true;
}

static void append_recorded_command(AutomationRecorder& recorder, const std::string& cmd)
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

static bool save_recording_snapshot(const AutomationRecorder& recorder)
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

struct AutomationPlayback
{
    bool active = false;
    bool rewind = false;  //after finish, rewind and play from start again
    std::vector<std::string> commands;
    size_t next_index = 0;
    int wait_frames = 0;
    std::string input_path = "automation_in.txt";
    std::string output_path = "automation_out.txt";
};

static std::string run_automation_command(
    AppState& app,
    const std::string& line,
    float viewport_w,
    float viewport_h,
    float fov_y_radians)
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

        if (viewport_w <= 0.0f || viewport_h <= 0.0f)
            return "error invalid_viewport";

        if (x < 0.0f || x > viewport_w || y < 0.0f || y > viewport_h)
        {
            app.picked_face = -1;
            return "ok pick_face_at_viewport miss";
        }

        Ray ray = make_camera_ray(app.camera, x, y, viewport_w, viewport_h, fov_y_radians);
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
    {
        return "picked_face " + std::to_string(app.picked_face);
    }

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

static bool try_load_automation_file(AutomationPlayback& playback)
{
    if (playback.active)
        return false;

    std::ifstream fin(playback.input_path);
    if (!fin.good()) {
        return false;
    }

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
    //std::remove(playback.input_path.c_str());

    if (playback.commands.empty())
        return false;

    playback.active = true;

    std::ofstream fout(playback.output_path, std::ios::app);
    fout << "# loaded " << playback.commands.size() << " commands\n";
    return true;
}

static void process_automation_step(
    AppState& app,
    AutomationPlayback& playback,
    float viewport_w,
    float viewport_h,
    float fov_y_radians)
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
        result = run_automation_command(app, line, viewport_w, viewport_h, fov_y_radians);
    }

    std::ofstream fout(playback.output_path, std::ios::app);
    fout << line << " => " << result << "\n";
}
//-------------------------------
static Ray make_camera_ray(
    const OrbitCamera& camera,
    float mouse_x,
    float mouse_y,
    float viewport_w,
    float viewport_h,
    float fov_y_radians)
{
    // Convert from viewport pixels to NDC [-1, 1]
    const float nx = 2.0f * (mouse_x / viewport_w) - 1.0f;
    const float ny = 1.0f - 2.0f * (mouse_y / viewport_h);

    const float aspect = viewport_w / viewport_h;
    const float tan_half_fov = std::tan(fov_y_radians * 0.5f);

    Vec3 forward = camera.forward();
    Vec3 right = camera.basis_right();
    Vec3 up = camera.basis_up();

    Vec3 dir = add(
        add(
            mul(right, nx * aspect * tan_half_fov),
            mul(up, ny * tan_half_fov)),
        forward);

    dir = normalize(dir);

    Ray ray;
    ray.origin = camera.position();
    ray.dir = dir;
    return ray;
}

static bool intersect_ray_unit_cube(const Ray& ray, int& out_face, float& out_t)
{
    const float bounds_min[3] = { -0.5f, -0.5f, -0.5f };
    const float bounds_max[3] = { 0.5f,  0.5f,  0.5f };

    float tmin = -1e30f;
    float tmax = 1e30f;
    int face_enter = -1;

    const float origin[3] = { ray.origin.x, ray.origin.y, ray.origin.z };
    const float dir[3] = { ray.dir.x, ray.dir.y, ray.dir.z };

    for (int axis = 0; axis < 3; ++axis)
    {
        if (std::fabs(dir[axis]) < 1e-6f)
        {
            if (origin[axis] < bounds_min[axis] || origin[axis] > bounds_max[axis])
                return false;
            continue;
        }

        float inv_d = 1.0f / dir[axis];
        float t1 = (bounds_min[axis] - origin[axis]) * inv_d;
        float t2 = (bounds_max[axis] - origin[axis]) * inv_d;

        int face1 = -1;
        int face2 = -1;

        // Face numbering:
        // 0 front  (+Z)
        // 1 back   (-Z)
        // 2 left   (-X)
        // 3 right  (+X)
        // 4 top    (+Y)
        // 5 bottom (-Y)

        if (axis == 0) { face1 = 2; face2 = 3; } // x
        if (axis == 1) { face1 = 5; face2 = 4; } // y
        if (axis == 2) { face1 = 1; face2 = 0; } // z

        if (t1 > t2)
        {
            std::swap(t1, t2);
            std::swap(face1, face2);
        }

        if (t1 > tmin)
        {
            tmin = t1;
            face_enter = face1;
        }

        tmax = std::min(tmax, t2);

        if (tmin > tmax)
            return false;
    }

    if (tmax < 0.0f)
        return false;

    out_t = (tmin >= 0.0f) ? tmin : tmax;
    out_face = face_enter;
    return true;
}

//-----------------------------------------------------------------
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

    bool start_record_mode = false;
    std::string start_record_path = "recorded_commands.txt";
    AutomationPlayback playback;
    for (int i = 1; i < narg; ++i)
    {
        std::string arg = argv[i];
        if (arg == "--replay" && i + 1 < narg)
        {
            playback.input_path = argv[++i];
            playback.active = false;
        }
        else if (arg == "--automation-out" && i + 1 < narg)
        {
            playback.output_path = argv[++i];
        }
        else if (arg == "--record")
        {
            start_record_mode = true;
            if (i + 1 < narg)
                start_record_path = argv[++i];
        }
    }

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

    //
    AppState app;

    //
    AutomationRecorder recorder;
    DragRecordingState drag_record;
    if (start_record_mode)
    {
        recorder.enabled = true;
        std::snprintf(recorder.path, sizeof(recorder.path), "%s", start_record_path.c_str());
        reset_recording_file(recorder);
    }

    float tint[3] = { 1.0f, 1.0f, 1.0f };
    //for face picking
    const char* face_names[6] = {
        "face 1 (front)",
        "face 2 (back)",
        "face 3 (left)",
        "face 4 (right)",
        "face 5 (top)",
        "face 6 (bottom)"
    };

    bool left_drag_started = false;
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

        if (app.picked_face >= 0)
            ImGui::Text("%s is picked", face_names[app.picked_face]);
        else
            ImGui::Text("No face is picked");

        if (ImGui::Button("Reset View"))
        {
            app.camera.reset();
            append_recorded_command(recorder, "reset_camera");
        }

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
        {
            save_recording_snapshot(recorder);
        }

        ImGui::Text("Recorded commands: %d", static_cast<int>(recorder.commands.size()));
        if (!recorder.last_command.empty())
            ImGui::Text("Last recorded: %s", recorder.last_command.c_str());

        ImGui::Text("Replay input: %s", playback.input_path.c_str());
        ImGui::Text("Replay output: %s", playback.output_path.c_str());
        if (playback.active)
            ImGui::Text("Replay active: step %d / %d", (int)playback.next_index, (int)playback.commands.size());
        else
            ImGui::Text("Replay idle");

        if (ImGui::Button("Load Replay Script"))
        {
            try_load_automation_file(playback);
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
            else if (viewport_hovered && !left_drag_started)
            {
                //click to pick a face
                ImVec2 mouse = ImGui::GetMousePos();
                float local_x = mouse.x - image_min.x;
                float local_y = mouse.y - image_min.y;

                if (local_x >= 0.0f && local_x <= avail.x &&
                    local_y >= 0.0f && local_y <= avail.y)
                {
                    Ray ray = make_camera_ray(
                        app.camera,
                        local_x,
                        local_y,
                        avail.x,
                        avail.y,
                        45.0f * PI / 180.0f);

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

        if (viewport_active && ImGui::IsMouseDragging(ImGuiMouseButton_Right))
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

        if (viewport_hovered && std::fabs(ImGui::GetIO().MouseWheel) > 0.0f)
        {
            const float dz = ImGui::GetIO().MouseWheel * 0.25f;
            app.camera.zoom(dz);

            std::ostringstream oss;
            oss << "zoom " << dz;
            append_recorded_command(recorder, oss.str());
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
        Mat4 view = app.camera.view_matrix();
        Mat4 mvp = multiply(proj, multiply(view, model));

        scene_shader.use();
        scene_shader.setUniformMat4("u_mvp", mvp.v);
        scene_shader.setUniform("tint", tint[0], tint[1], tint[2]);

        //by face
        glBindVertexArray(vao);

        for (int face = 0; face < 6; ++face)
        {
            if (face == app.picked_face)
                scene_shader.setUniform("tint", 1.0f, 1.0f, 0.2f); // highlight
            else
                scene_shader.setUniform("tint", tint[0], tint[1], tint[2]);

            glDrawArrays(GL_TRIANGLES, face * 6, 6);
        }

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

        // test automation replay
        if (!playback.active && playback.rewind)
            try_load_automation_file(playback);

        process_automation_step(
            app,
            playback,
            static_cast<float>(viewport_fb_w),
            static_cast<float>(viewport_fb_h),
            45.0f * PI / 180.0f);
    }  //while loop
    if (recorder.enabled)
        save_recording_snapshot(recorder);

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
