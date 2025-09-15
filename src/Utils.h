#pragma once

#include <stdint.h>

#include "Structs.h"
#include "Matrix.h"

namespace math
{
    inline static const float pi = 3.14159265359f;
    inline static const float e = 2.71828f;
}

class Utils
{
public:
    static const Vec3f hexToRGB(const uint32_t colour);
    static const uint32_t rgbToHex(const Vec3f& colour);
    static const uint8_t linearToSRGB(float colour);
    static const uint32_t combineHex(const uint32_t& ca, const uint32_t& cb, const float ratio);
    static const uint32_t rasteriseColour(const uint32_t c0, const uint32_t c1, const uint32_t c2, const float w0, const float w1, const float w2);
    static const uint32_t applyLighting(const uint32_t baseColour, const float intensity);
    static const uint32_t pcgHash(const uint32_t seed);

    static const Vec3f reflect(const Vec3f& i, const Vec3f& n);
    static const Vec3f getFaceNormal(const Vec3f& a, const Vec3f& b, const Vec3f& c);
    static const Vec3f normalise(const Vec3f& vec);
    static const float length(const Vec3f& normal);
    static const Vec3f cross(const Vec3f& a, const Vec3f& b);
    static const float dot(const Vec3f& a, const Vec3f& b);

    static const float degToRad(const float& deg);
    static const Vec4f toVec4(const Vec3f& vec, float w = 1.f);
    static const Vec3f toVec3(const Vec4f& vec);

    static const float lerp(const float& a, const float& b, float t);

    static const void drawLine(const Vertex& v1, const Vertex& v2);

private:
    static const void drawLineLow(const Vertex& v1, const Vertex& v2);
    static const void drawLineHigh(const Vertex& v1, const Vertex& v2);
};