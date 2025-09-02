#pragma once

#include <stdint.h>
#include <vector>

struct Vec2i
{
    int32_t x, y;
};

struct Vec3i
{
    int32_t x, y, z;
};

struct Vec2f
{
    float x, y;

    const Vec2f operator+(const Vec2f& rhs) const { return { x + rhs.x, y + rhs.y }; }
    const Vec2f operator-(const Vec2f& rhs) const { return { x - rhs.x, y - rhs.y }; }
    const Vec2f operator*(const Vec2f& rhs) const { return { x * rhs.x, y * rhs.y }; }
    const Vec2f operator/(const Vec2f& rhs) const { return { x / rhs.x, y / rhs.y }; }

    const Vec2f operator+(const float& value) const { return { x + value, y + value }; }
    const Vec2f operator-(const float& value) const { return { x - value, y - value }; }
    const Vec2f operator*(const float& value) const { return { x * value, y * value }; }
    const Vec2f operator/(const float& value) const { return { x / value, y / value }; }
};

struct Vec3f
{
    float x, y, z;

    const Vec3f operator+(const Vec3f& rhs) const { return { x + rhs.x, y + rhs.y, z + rhs.z }; }
    const Vec3f operator-(const Vec3f& rhs) const { return { x - rhs.x, y - rhs.y, z - rhs.z }; }
    const Vec3f operator*(const Vec3f& rhs) const { return { x * rhs.x, y * rhs.y, z * rhs.z }; }
    const Vec3f operator/(const Vec3f& rhs) const { return { x / rhs.x, y / rhs.y, z / rhs.z }; }

    const Vec3f operator+(const float& value) const { return { x + value, y + value, z + value }; }
    const Vec3f operator-(const float& value) const { return { x - value, y - value, z - value }; }
    const Vec3f operator*(const float& value) const { return { x * value, y * value, z * value }; }
    const Vec3f operator/(const float& value) const { return { x / value, y / value, z / value }; }
};

struct Vec4f
{
    float x, y, z, w;
};

struct Vertex
{
    Vec3f position;
    Vec3f normals;
    Vec2f texCoords;
    uint32_t colour;
};