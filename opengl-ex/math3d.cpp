#include "math3d.h"

Vec3 make_vec3(float x, float y, float z)
{
    return { x, y, z };
}

Vec3 add(const Vec3& a, const Vec3& b)
{
    return { a.x + b.x, a.y + b.y, a.z + b.z };
}

Vec3 sub(const Vec3& a, const Vec3& b)
{
    return { a.x - b.x, a.y - b.y, a.z - b.z };
}

Vec3 mul(const Vec3& v, float s)
{
    return { v.x * s, v.y * s, v.z * s };
}

float dot(const Vec3& a, const Vec3& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3 cross(const Vec3& a, const Vec3& b)
{
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

float length(const Vec3& v)
{
    return std::sqrt(dot(v, v));
}

Vec3 normalize(const Vec3& v)
{
    float len = length(v);
    if (len < 1e-6f)
        return { 0.0f, 0.0f, 0.0f };
    return { v.x / len, v.y / len, v.z / len };
}

Vec3 negate(const Vec3& v)
{
    return { -v.x, -v.y, -v.z };
}

Mat4 identity()
{
    Mat4 m = {};
    m.v[0] = 1.0f;
    m.v[5] = 1.0f;
    m.v[10] = 1.0f;
    m.v[15] = 1.0f;
    return m;
}

Mat4 multiply(const Mat4& a, const Mat4& b)
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

Mat4 perspective(float fov_y_radians, float aspect, float z_near, float z_far)
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

Mat4 translate(float x, float y, float z)
{
    Mat4 m = identity();
    m.v[12] = x;
    m.v[13] = y;
    m.v[14] = z;
    return m;
}

Mat4 rotate_x(float angle)
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

Mat4 rotate_y(float angle)
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

Mat4 inverse_rigid_body(const Mat4& m)
{
    Mat4 out = identity();

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

Vec3 transform_point(const Mat4& m, const Vec3& p)
{
    return {
        m.v[0] * p.x + m.v[4] * p.y + m.v[8] * p.z + m.v[12],
        m.v[1] * p.x + m.v[5] * p.y + m.v[9] * p.z + m.v[13],
        m.v[2] * p.x + m.v[6] * p.y + m.v[10] * p.z + m.v[14]
    };
}

Vec3 transform_vector(const Mat4& m, const Vec3& v)
{
    return {
        m.v[0] * v.x + m.v[4] * v.y + m.v[8] * v.z,
        m.v[1] * v.x + m.v[5] * v.y + m.v[9] * v.z,
        m.v[2] * v.x + m.v[6] * v.y + m.v[10] * v.z
    };
}

Mat4 look_at(const Vec3& eye, const Vec3& target, const Vec3& up)
{
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
