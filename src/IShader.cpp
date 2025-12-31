#include "IShader.h"

#include <cmath>

Vec4f IShader::vec4(const Vec3f &vec, const float w)
{
    return { vec.x, vec.y, vec.z, w };
}

Vec4f IShader::vec4(const Vec2f &vec, const float z, const float w)
{
    return { vec.x, vec.y, z, w };
}

float IShader::dot(const Vec3f& a, const Vec3f& b)
{
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

float IShader::length(const Vec3f& normal)
{
    return sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
}

Vec3f IShader::cross(const Vec3f& a, const Vec3f& b)
{
    Vec3f c;
    c.x = a.y * b.z - a.z * b.y;
    c.y = a.z * b.x - a.x * b.z;
    c.z = a.x * b.y - a.y * b.x;

    return c;
}

Vec3f IShader::reflect(const Vec3f& i, const Vec3f& n)
{
    return i - n * (2.0f * dot(n, i));
}

Vec3f IShader::normalise(const Vec3f& vec)
{
    float mag = sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
    return { vec.x / mag, vec.y / mag, vec.z / mag };
}