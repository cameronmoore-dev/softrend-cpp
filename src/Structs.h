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
    UINT colour;
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