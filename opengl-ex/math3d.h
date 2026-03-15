#pragma once

#include <cmath>

struct Vec3
{
    float x, y, z;
};

struct Vec4
{
    float x, y, z, w;
};

struct Mat4
{
    float v[16];
};

Vec3 make_vec3(float x, float y, float z);
Vec3 add(const Vec3& a, const Vec3& b);
Vec3 sub(const Vec3& a, const Vec3& b);
Vec3 mul(const Vec3& v, float s);
float dot(const Vec3& a, const Vec3& b);
Vec3 cross(const Vec3& a, const Vec3& b);
float length(const Vec3& v);
Vec3 normalize(const Vec3& v);
Vec3 negate(const Vec3& v);

Mat4 identity();
Mat4 multiply(const Mat4& a, const Mat4& b);
Mat4 perspective(float fov_y_radians, float aspect, float z_near, float z_far);
Mat4 translate(float x, float y, float z);
Mat4 rotate_x(float angle);
Mat4 rotate_y(float angle);
Mat4 inverse_rigid_body(const Mat4& m);
Vec3 transform_point(const Mat4& m, const Vec3& p);
Vec3 transform_vector(const Mat4& m, const Vec3& v);
Mat4 look_at(const Vec3& eye, const Vec3& target, const Vec3& up);
