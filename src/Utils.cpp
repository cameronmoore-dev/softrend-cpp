#include "Utils.h"

UINT Utils::lerpHex(UINT a, UINT b, FLOAT t)
{
    INT a_r = (a >> 16) & 0xff;
    INT a_g = (a >> 8) & 0xff;
    INT a_b = a & 0xff;

    INT b_r = (b >> 16) & 0xff;
    INT b_g = (b >> 8) & 0xff;
    INT b_b = b & 0xff;

    UINT l_r = a_r + (b_r - a_r) * t;
    UINT l_g = a_g + (b_g - a_g) * t;
    UINT l_b = a_b + (b_b - a_b) * t;

    return (l_r << 16) | (l_g << 8) | l_b;
}

UINT Utils::rasteriseColour(UINT c0, UINT c1, UINT c2, FLOAT w0, FLOAT w1, FLOAT w2)
{
    INT c0_r = (c0 >> 16) & 0xff;
    INT c0_g = (c0 >> 8) & 0xff;
    INT c0_b = c0 & 0xff;

    INT c1_r = (c1 >> 16) & 0xff;
    INT c1_g = (c1 >> 8) & 0xff;
    INT c1_b = c1 & 0xff;

    INT c2_r = (c2 >> 16) & 0xff;
    INT c2_g = (c2 >> 8) & 0xff;
    INT c2_b = c2 & 0xff;

    UINT r = (w0 * c0_r) + (w1 * c1_r) + (w2 * c2_r);
    UINT g = (w0 * c0_g) + (w1 * c1_g) + (w2 * c2_g);
    UINT b = (w0 * c0_b) + (w1 * c1_b) + (w2 * c2_b);

    return (r << 16) | (g << 8) | b;
}

UINT Utils::pcgHash(UINT seed)
{
    UINT state = seed * 747796405u + 2891336453u;
    UINT word = ((state >> ((state >> 28u) + 4u)) ^ state);
    return (word >> 22u) ^ word;
}

Vec3f Utils::cross(const Vec3f& a, const Vec3f& b)
{
    Vec3f c;
    c.x = a.y * b.z - a.z * b.y;
    c.y = a.z * b.x - a.x * b.z;
    c.z = a.x * b.y - a.y * b.x;

    return c;
}

FLOAT Utils::dot(const Vec3i& a, const Vec3i& b)
{
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

FLOAT Utils::degToRad(const FLOAT& deg)
{
    return deg * (pi / 180.f);
}

Vec4f Utils::toVec4(const Vec3f& vec, FLOAT w)
{
    return { vec.x, vec.y, vec.z, w };
}

Vec3f Utils::screenToClip(const Vec3f& pos, INT width, INT height)
{
    FLOAT xClip = (pos.x / (width * 0.5f)) - 1.f;
    FLOAT yClip = -(pos.y / (height * 0.5f)) + 1.f;
    return { xClip, yClip, pos.z };
}

void Utils::drawLine(const Vertex& v1, const Vertex& v2)
{
    if (std::abs(v2.position.y - v1.position.y) < std::abs(v2.position.x - v1.position.x))
    {
        if (v1.position.x > v2.position.x)
        {
            drawLineLow(v2, v1);
        }
        else
        {
            drawLineLow(v1, v2);
        }
    }
    else  
    {
        if (v1.position.y > v2.position.y)
        {
            drawLineHigh(v2, v1);
        }
        else  
        {
            drawLineHigh(v1, v2);
        }
    }
}

void Utils::drawLineLow(const Vertex& v1, const Vertex& v2)
{
    INT dx = v2.position.x - v1.position.x;
    INT dy = v2.position.y - v1.position.y;
    INT yi = 1;

    if (dy < 0)
    {
        yi = -1;
        dy = -dy;
    }

    INT d = 2 * dy - dx;
    INT y = v1.position.y;

    for (INT x = v1.position.x; x < v2.position.x; x++)
    {
        if (d > 0)
        {
            y += yi;
            d += 2 * (dy-dx);
        }
        else
        {
            d += 2 * dy;
        }
    }
}

void Utils::drawLineHigh(const Vertex& v1, const Vertex& v2)
{
    INT dx = v2.position.x - v1.position.x;
    INT dy = v2.position.y - v1.position.y;
    INT xi = 1;
    if (dx < 0)
    {
        xi = -1;
        dx = -dx;
    }
    INT d = 2 * dx - dy;
    INT x = v1.position.x;

    for (INT y = v1.position.y; y < v2.position.y; y++)
    {
        if (d > 0)
        {
            x += xi;
            d += 2 * (dx-dy);
        }
        else
        {
            d += 2 * dx;
        }
    }
}