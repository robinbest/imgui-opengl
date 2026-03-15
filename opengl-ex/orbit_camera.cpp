#include "orbit_camera.h"

#include <algorithm>
#include <cmath>

Vec3 OrbitCamera::position() const
{
    const float cp = std::cos(pitch);
    const float sp = std::sin(pitch);
    const float cy = std::cos(yaw);
    const float sy = std::sin(yaw);

    Vec3 offset = {
        distance * cp * sy,
        distance * sp,
        distance * cp * cy
    };

    return add(target, offset);
}

Vec3 OrbitCamera::forward() const
{
    return normalize(sub(target, position()));
}

Vec3 OrbitCamera::right() const
{
    return normalize(cross(forward(), world_up()));
}

Vec3 OrbitCamera::up() const
{
    return normalize(cross(right(), forward()));
}

Vec3 OrbitCamera::basis_right() const
{
    return right();
}

Vec3 OrbitCamera::basis_up() const
{
    return up();
}

Vec3 OrbitCamera::world_up() const
{
    return { 0.0f, 1.0f, 0.0f };
}

Mat4 OrbitCamera::view_matrix() const
{
    return look_at(position(), target, up());
}

void OrbitCamera::orbit(float dx, float dy)
{
    yaw += dx;
    pitch += dy;

    const float pitch_limit = 1.45f;
    pitch = std::clamp(pitch, -pitch_limit, pitch_limit);
}

void OrbitCamera::zoom(float delta)
{
    distance -= delta;
    distance = std::clamp(distance, 1.5f, 20.0f);
}

void OrbitCamera::pan(float dx, float dy)
{
    const Vec3 r = right();
    const Vec3 u = up();
    target = add(target, add(mul(r, dx), mul(u, dy)));
}

void OrbitCamera::reset()
{
    target = { 0.0f, 0.0f, 0.0f };
    yaw = 0.6f;
    pitch = 0.4f;
    distance = 3.0f;
}
