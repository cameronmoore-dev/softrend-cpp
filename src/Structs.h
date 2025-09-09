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

    const Vec2f operator*=(const float& value) { return {x *= value, y *= value }; }
};

struct Vec4f
{
    float x, y, z, w;

    const Vec4f operator+(const Vec4f& rhs) const { return { x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w }; }
    const Vec4f operator-(const Vec4f& rhs) const { return { x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w}; }
    const Vec4f operator*(const Vec4f& rhs) const { return { x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w }; }
    const Vec4f operator/(const Vec4f& rhs) const { return { x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w }; }

    const Vec4f operator+(const float& value) const { return { x + value, y + value, z + value }; }
    const Vec4f operator-(const float& value) const { return { x - value, y - value, z - value }; }
    const Vec4f operator*(const float& value) const { return { x * value, y * value, z * value }; }
    const Vec4f operator/(const float& value) const { return { x / value, y / value, z / value }; }

    const Vec4f operator+=(const Vec4f& rhs) { return { x += rhs.x, y += rhs.y, z += rhs.z }; }
    const Vec4f operator-=(const Vec4f& rhs) { return { x -= rhs.x, y -= rhs.y, z -= rhs.z }; }
    
    const Vec4f operator/=(const float& rhs) { return { x /= rhs, y /= rhs, z /= rhs }; }

    const Vec4f operator-() const
    {
        return { -x, -y, -z };
    }
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

    const Vec3f operator+=(const Vec3f& rhs) { return { x += rhs.x, y += rhs.y, z += rhs.z }; }
    const Vec3f operator-=(const Vec3f& rhs) { return { x -= rhs.x, y -= rhs.y, z -= rhs.z }; }
    
    const Vec3f operator/=(const float& rhs) { return { x /= rhs, y /= rhs, z /= rhs }; }

    const Vec3f operator-() const
    {
        return { -x, -y, -z };
    }
};

struct Vertex
{
    Vec3f position;
    Vec3f normal;
    Vec2f texCoords;
    uint32_t colour;
};

struct glVertex
{
    Vec4f position;
    Vec3f normal;
    Vec2f texCoords;
    uint32_t colour;

    float inverseW;
};

struct glTriangle
{
    glVertex v0, v1, v2;
};