#pragma once

#include "math3d.h"

struct OrbitCamera
{
    Vec3 target = { 0.0f, 0.0f, 0.0f };
    float yaw = 0.6f;
    float pitch = 0.4f;
    float distance = 3.0f;

    Vec3 position() const;
    Vec3 forward() const;
    Vec3 right() const;
    Vec3 up() const;
    Vec3 basis_right() const;
    Vec3 basis_up() const;
    Vec3 world_up() const;
    Mat4 view_matrix() const;

    void orbit(float dx, float dy);
    void zoom(float delta);
    void pan(float dx, float dy);
    void reset();
};
