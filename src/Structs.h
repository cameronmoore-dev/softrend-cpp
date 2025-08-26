#pragma once

#include <minwindef.h>
#include <vector>

struct FrameBuffer
{
    INT width = 0;
    INT height = 0;
    UINT* pixels = 0;
};

struct Vec2i
{
    INT x, y;
};

struct Vec3i
{
    INT x, y, z;
};

struct Vec2f
{
    FLOAT x, y;

    Vec2f add(const Vec2f& other) const
    {
        return { x + other.x, y + other.y };
    }

    Vec2f add(const FLOAT& other) const
    {
        return { x + other, y + other };
    }

    Vec2f multiply(const Vec2f& other) const
    {
        return { x * other.x, y * other.y };
    }

    Vec2f multiply(const FLOAT& other) const
    {
        return { x * other, y * other };
    }

    Vec2f divide(const FLOAT& other) const
    {
        return { x / other, y / other };
    }

    Vec2f divide(const Vec2f& other) const
    {
        return { x / other.x, y / other.y };
    }

    Vec2f operator*(const Vec2f& other) const
    {
        return multiply(other);
    }

    Vec2f operator*(const FLOAT& other) const
    {
        return multiply(other);
    }

    Vec2f operator+(const Vec2f& other) const
    {
        return add(other);
    }

    Vec2f operator+(const FLOAT& other) const
    {
        return add(other);
    }

    Vec2f operator/(const FLOAT& other) const
    {
        return divide(other);
    }

    Vec2f operator/(const Vec2f& other) const
    {
        return divide(other);
    }
};

struct Vec3f
{
    FLOAT x, y, z;

    Vec3f operator-()
    {
        return { -x, -y, -z };
    }

    const Vec3f operator-(const Vec3f& rhs) const
    {
        return { x - rhs.x, y - rhs.y, z - rhs.z };
    }
};

struct Vec4f
{
    FLOAT x, y, z, w;
};

struct Vertex
{
    Vec3f position;
    Vec3f normals;
    Vec2f texCoords;
    UINT colour;

    FLOAT invW;
    Vec2f uvOverW;
};

struct FaceIndex
{
    INT positionIndex;
    INT texCoordIndex;
    INT normalIndex;
};

struct Face
{
    std::vector<FaceIndex> indices;
};