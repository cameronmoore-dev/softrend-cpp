#pragma once

#include <cstdint>
#include <cmath>
#include <algorithm>

#include "Structs.h"
#include "Matrix.h"

namespace math
{
    inline const float pi = 3.14159265359f;
    inline const float e = 2.71828f;

    const float lerp(const float a, const float b, float t);
    const float degToRad(const float deg);
    const float dot(const Vec3f& a, const Vec3f& b);
    const float length(const Vec3f& normal);
    const Vec3f normalise(const Vec3f& vec);
    const Vec3f cross(const Vec3f& a, const Vec3f& b);
    const uint8_t linearToSRGB(float channel);
    const uint32_t rgbToHex(const Vec3f& colour);
    const Vec3f hexToRGB(const uint32_t colour);
}