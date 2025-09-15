#include "Utils.h"

#include <cmath>
#include <algorithm>
#include <stdio.h>

const Vec3f Utils::hexToRGB(const uint32_t colour)
{
    float r = float((colour >> 16) & 0xFF) / 255.f;
    float g = float((colour >> 8) & 0xFF) / 255.f;
    float b = float(colour & 0xFF) / 255.f;

    return  { r, g, b };
}

const uint32_t Utils::rgbToHex(const Vec3f& colour)
{
    int R = std::clamp(int(colour.x * 255.f), 0, 255);
    int G = std::clamp(int(colour.y * 255.f), 0, 255);
    int B = std::clamp(int(colour.z * 255.f), 0, 255);

    return (R << 16) | (G << 8) | B;
}

const uint8_t Utils::linearToSRGB(float channel)
{
    channel = std::clamp(channel, 0.0f, 1.0f);
    return (std::powf(channel, 1.f / 2.2f) * 255.f) + 0.5f;
}

const uint32_t Utils::combineHex(const uint32_t& ca, const uint32_t& cb, const float ratio)
{
    uint8_t r = ((ca >> 16) & 0xFF) * (1-ratio) + ((cb >> 16) & 0xFF) * ratio;
    uint8_t g = ((ca >> 8)  & 0xFF) * (1-ratio) + ((cb >> 8)  & 0xFF) * ratio;
    uint8_t b = ((ca)       & 0xFF) * (1-ratio) + ((cb)       & 0xFF) * ratio;

    return (0xFF << 24) | (r << 16) | (g << 8) | b;
}

const uint32_t Utils::rasteriseColour(uint32_t c0, uint32_t c1, uint32_t c2, float w0, float w1, float w2)
{
    int32_t c0_r = (c0 >> 16) & 0xff;
    int32_t c0_g = (c0 >> 8) & 0xff;
    int32_t c0_b = c0 & 0xff;

    int32_t c1_r = (c1 >> 16) & 0xff;
    int32_t c1_g = (c1 >> 8) & 0xff;
    int32_t c1_b = c1 & 0xff;

    int32_t c2_r = (c2 >> 16) & 0xff;
    int32_t c2_g = (c2 >> 8) & 0xff;
    int32_t c2_b = c2 & 0xff;

    uint32_t r = (w0 * c0_r) + (w1 * c1_r) + (w2 * c2_r);
    uint32_t g = (w0 * c0_g) + (w1 * c1_g) + (w2 * c2_g);
    uint32_t b = (w0 * c0_b) + (w1 * c1_b) + (w2 * c2_b);

    return (r << 16) | (g << 8) | b;
}

const uint32_t Utils::pcgHash(uint32_t seed)
{
    uint32_t state = seed * 747796405u + 2891336453u;
    uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state);
    return (word >> 22u) ^ word;
}

const uint32_t Utils::applyLighting(uint32_t baseColour, float intensity)
{
    int32_t r = ((baseColour >> 16) & 0xff) * intensity;
    int32_t g = ((baseColour >> 8) & 0xff) * intensity;
    int32_t b = (baseColour & 0xff) * intensity;

    return (r << 16) | (g << 8) | b;
}

const Vec3f Utils::reflect(const Vec3f& i, const Vec3f& n)
{
    return i - n * (2.0f * dot(n, i));
}

const Vec3f Utils::getFaceNormal(const Vec3f& a, const Vec3f& b, const Vec3f& c)
{
    Vec3f ab;
    Vec3f ac;

    ab = b - a;
    ac = c - a;

    return normalise(cross(ac, ab));
}

const Vec3f Utils::normalise(const Vec3f& vec)
{
        float mag = sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
        return { vec.x / mag, vec.y / mag, vec.z / mag };
}

const float Utils::length(const Vec3f& normal)
{
    return sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
}

const Vec3f Utils::cross(const Vec3f& a, const Vec3f& b)
{
    Vec3f c;
    c.x = a.y * b.z - a.z * b.y;
    c.y = a.z * b.x - a.x * b.z;
    c.z = a.x * b.y - a.y * b.x;

    return c;
}

const float Utils::dot(const Vec3f& a, const Vec3f& b)
{
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

const float Utils::degToRad(const float& deg)
{
    return deg * (math::pi / 180.f);
}

const Vec4f Utils::toVec4(const Vec3f& vec, float w)
{
    return { vec.x, vec.y, vec.z, w };
}

const Vec3f Utils::toVec3(const Vec4f& vec)
{
    return { vec.x, vec.y, vec.z };
}

const float Utils::lerp(const float& a, const float& b, float t)
{
    return a + t * (b-a);
}

const void Utils::drawLine(const Vertex& v1, const Vertex& v2)
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

const void Utils::drawLineLow(const Vertex& v1, const Vertex& v2)
{
    int32_t dx = v2.position.x - v1.position.x;
    int32_t dy = v2.position.y - v1.position.y;
    int32_t yi = 1;

    if (dy < 0)
    {
        yi = -1;
        dy = -dy;
    }

    int32_t d = 2 * dy - dx;
    int32_t y = v1.position.y;

    for (int32_t x = v1.position.x; x < v2.position.x; x++)
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

const void Utils::drawLineHigh(const Vertex& v1, const Vertex& v2)
{
    int32_t dx = v2.position.x - v1.position.x;
    int32_t dy = v2.position.y - v1.position.y;
    int32_t xi = 1;
    if (dx < 0)
    {
        xi = -1;
        dx = -dx;
    }
    int32_t d = 2 * dx - dy;
    int32_t x = v1.position.x;

    for (int32_t y = v1.position.y; y < v2.position.y; y++)
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