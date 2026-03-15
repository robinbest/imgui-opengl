#include "picking.h"

#include <algorithm>
#include <cmath>

Ray make_camera_ray(
    const OrbitCamera& camera,
    float mouse_x,
    float mouse_y,
    float viewport_w,
    float viewport_h,
    float fov_y_radians)
{
    const float nx = 2.0f * (mouse_x / viewport_w) - 1.0f;
    const float ny = 1.0f - 2.0f * (mouse_y / viewport_h);

    const float aspect = viewport_w / viewport_h;
    const float tan_half_fov = std::tan(fov_y_radians * 0.5f);

    const Vec3 forward = camera.forward();
    const Vec3 right = camera.basis_right();
    const Vec3 up = camera.basis_up();

    Vec3 dir = add(
        add(
            mul(right, nx * aspect * tan_half_fov),
            mul(up, ny * tan_half_fov)),
        forward);

    dir = normalize(dir);

    return { camera.position(), dir };
}

bool intersect_ray_unit_cube(const Ray& ray, int& out_face, float& out_t)
{
    const float bounds_min[3] = { -0.5f, -0.5f, -0.5f };
    const float bounds_max[3] = {  0.5f,  0.5f,  0.5f };

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

        if (axis == 0) { face1 = 2; face2 = 3; }
        if (axis == 1) { face1 = 5; face2 = 4; }
        if (axis == 2) { face1 = 1; face2 = 0; }

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
