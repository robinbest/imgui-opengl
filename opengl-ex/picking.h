#pragma once

#include "orbit_camera.h"

struct Ray
{
    Vec3 origin;
    Vec3 dir;
};

Ray make_camera_ray(
    const OrbitCamera& camera,
    float mouse_x,
    float mouse_y,
    float viewport_w,
    float viewport_h,
    float fov_y_radians);

bool intersect_ray_unit_cube(const Ray& ray, int& out_face, float& out_t);
