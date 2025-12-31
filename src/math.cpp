#include "math.h"

namespace math
{
    const float lerp(const float a, const float b, float t)
    {
        return a + t * (b-a);
    }

    const float degToRad(const float deg)
    {
        return deg * (pi / 180.f);
    }

    const float dot(const Vec3f& a, const Vec3f& b)
    {
        return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
    }

    const float length(const Vec3f& normal)
    {
        return sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
    }

    const Vec3f normalise(const Vec3f& vec)
    {
        float mag = sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
        return { vec.x / mag, vec.y / mag, vec.z / mag };
    }

    const Vec3f cross(const Vec3f& a, const Vec3f& b)
    {
        Vec3f c;
        c.x = a.y * b.z - a.z * b.y;
        c.y = a.z * b.x - a.x * b.z;
        c.z = a.x * b.y - a.y * b.x;

        return c;
    }

    const uint8_t linearToSRGB(float channel)
    {
        channel = std::clamp(channel, 0.0f, 1.0f);
        return (std::powf(channel, 1.f / 2.2f) * 255.f) + 0.5f;
    }

    const uint32_t rgbToHex(const Vec3f& colour) 
    {
        int R = std::clamp(int(colour.x * 255.f), 0, 255);
        int G = std::clamp(int(colour.y * 255.f), 0, 255);
        int B = std::clamp(int(colour.z * 255.f), 0, 255);
        return (R << 16) | (G << 8) | B;
    }

    const Vec3f hexToRGB(const uint32_t colour)
    {
        float r = float((colour >> 16) & 0xFF) / 255.f;
        float g = float((colour >> 8) & 0xFF) / 255.f;
        float b = float(colour & 0xFF) / 255.f;
        return  { r, g, b };
    }
}