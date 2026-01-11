#pragma once

#include <array>
#include <cstdint>
#include <cmath>
#include <algorithm>

namespace echo
{
    using mat4 = std::array<std::array<float, 4>, 4>;
    using mat3 = std::array<std::array<float, 3>, 3>;

    inline const float pi = 3.14159265359f;
    inline const float e = 2.71828f;

    struct rect
    {
        int32_t x = 0;
        int32_t y = 0;
        uint16_t width;
        uint16_t height;
    };
    
    struct vec2i
    {
        int32_t x = 0;
        int32_t y = 0;
    };

    struct vec4f
    {
        float x = 0.f;
        float y = 0.f;
        float z = 0.f;
        float w = 0.f;

        const vec4f operator+(const vec4f &rhs) const { return { x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w }; }
        const vec4f operator-(const vec4f &rhs) const { return { x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w }; }
        const vec4f operator*(const vec4f &rhs) const { return { x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w }; }
        const vec4f operator/(const vec4f &rhs) const { return { x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w }; }

        const vec4f operator+(const float &value) const { return { x + value, y + value, z + value, w + value }; }
        const vec4f operator-(const float &value) const { return { x - value, y - value, z - value, w - value }; }
        const vec4f operator*(const float &value) const { return { x * value, y * value, z * value, w * value }; }
        const vec4f operator/(const float &value) const { return { x / value, y / value, z / value, w / value }; }

        const vec4f operator+=(const vec4f &rhs) { return { x += rhs.x, y += rhs.y, z += rhs.z, w += rhs.w}; }
        const vec4f operator-=(const vec4f &rhs) { return { x -= rhs.x, y -= rhs.y, z -= rhs.z, w -= rhs.w }; }
        
        const vec4f operator/=(const float &rhs) { return { x /= rhs, y /= rhs, z /= rhs }; }

        const vec4f operator-() const
        {
            return { -x, -y, -z };
        }
    };

    struct vec2f
    {
        float x = 0.f;
        float y = 0.f;

        const vec2f operator+(const vec2f &rhs) const { return { x + rhs.x, y + rhs.y }; }
        const vec2f operator-(const vec2f &rhs) const { return { x - rhs.x, y - rhs.y }; }
        const vec2f operator*(const vec2f &rhs) const { return { x * rhs.x, y * rhs.y }; }
        const vec2f operator/(const vec2f &rhs) const { return { x / rhs.x, y / rhs.y }; }

        const vec2f operator+(const vec4f &rhs) const { return { x + rhs.x, y + rhs.y }; }
        const vec2f operator-(const vec4f &rhs) const { return { x - rhs.x, y - rhs.y }; }
        const vec2f operator*(const vec4f &rhs) const { return { x * rhs.x, y * rhs.y }; }
        const vec2f operator/(const vec4f &rhs) const { return { x / rhs.x, y / rhs.y }; }

        const vec2f operator+(const float &value) const { return { x + value, y + value }; }
        const vec2f operator-(const float &value) const { return { x - value, y - value }; }
        const vec2f operator*(const float &value) const { return { x * value, y * value }; }
        const vec2f operator/(const float &value) const { return { x / value, y / value }; }

        const vec2f operator+=(const float &value) { return {x += value, y += value }; }
        const vec2f operator-=(const float &value) { return {x -= value, y -= value }; }
        const vec2f operator*=(const float &value) { return {x *= value, y *= value }; }
    };

    struct vec3f
    {
        float x = 0.f;
        float y = 0.f;
        float z = 0.f;

        const vec3f operator+(const vec3f &rhs) const { return { x + rhs.x, y + rhs.y, z + rhs.z }; }
        const vec3f operator-(const vec3f &rhs) const { return { x - rhs.x, y - rhs.y, z - rhs.z }; }
        const vec3f operator*(const vec3f &rhs) const { return { x * rhs.x, y * rhs.y, z * rhs.z }; }
        const vec3f operator/(const vec3f &rhs) const { return { x / rhs.x, y / rhs.y, z / rhs.z }; }

        const vec3f operator+(const vec4f &rhs) const { return { x + rhs.x, y + rhs.y, z + rhs.z }; }
        const vec3f operator-(const vec4f &rhs) const { return { x - rhs.x, y - rhs.y, z - rhs.z }; }
        const vec3f operator*(const vec4f &rhs) const { return { x * rhs.x, y * rhs.y, z * rhs.z }; }
        const vec3f operator/(const vec4f &rhs) const { return { x / rhs.x, y / rhs.y, z / rhs.z }; }

        const vec3f operator+(const float &value) const { return { x + value, y + value, z + value }; }
        const vec3f operator-(const float &value) const { return { x - value, y - value, z - value }; }
        const vec3f operator*(const float &value) const { return { x * value, y * value, z * value }; }
        const vec3f operator/(const float &value) const { return { x / value, y / value, z / value }; }

        const vec3f operator+=(const vec3f &rhs) { return { x += rhs.x, y += rhs.y, z += rhs.z }; }
        const vec3f operator-=(const vec3f &rhs) { return { x -= rhs.x, y -= rhs.y, z -= rhs.z }; }
        const vec3f operator*=(const vec3f &rhs) { return { x += rhs.x, y += rhs.y, z += rhs.z }; }
        
        const vec3f operator*=(const float &value) { return { x *= value, y *= value, z *= value }; }
        const vec3f operator/=(const float &value) { return { x /= value, y /= value, z /= value }; }
        const vec3f operator-=(const float &value) { return { x -= value, y -= value, z -= value }; }

        const vec3f operator-() const
        {
            return { -x, -y, -z };
        }
    };

    const float lerp(const float a, const float b, float t);
    const float degToRad(const float deg);
    const float dot(const vec3f &a, const vec3f &b);
    const float length(const vec3f &normal);
    const vec3f normalise(const vec3f &vec);
    const vec3f cross(const vec3f &a, const vec3f &b);
    const vec4f srgbToLinear(const vec4f &colour);
    const vec4f linearToSRGB(const vec4f &colour);
    const uint32_t rgbaToHex(const vec4f &colour);
    const uint32_t rgbToHex(const vec3f &colour);
    const vec4f hexToRGBA(const uint32_t colour);
    const vec3f hexToRGB(const uint32_t colour);
    vec3f vec3(const vec4f &vec);
    vec4f vec4(const vec3f &vec, const float w);
    vec4f vec4(const vec2f &vec, const float z, const float w);
    
    const mat3 constructTBN(const vec3f &t, const vec3f &b, const vec3f &n);
    const mat4 perspective(const float fov, const float aspect, const float zNear, const float zFar);
    const mat4 orthographic(const int32_t left, const int32_t right, const int32_t bottom, const int32_t top, const float zNear, const float zFar);
    const mat4 lookAt(const vec3f& pos, const vec3f& forward, const vec3f& up);

    const mat4 translate(const vec3f& vec);
    const mat4 rotate(const float angle, const vec3f &axis);
    const mat4 scale(const vec3f& vec);

    const mat4 identity();

    namespace _priv
    {
        const mat4 multiply(const mat4 &matA, const mat4 &matB);
        const vec4f multiply(const mat4 &mat, const vec4f &vec);
        const vec3f multiplyVec3(const mat4 &mat, const vec3f &vec);
        const vec3f multiplyVec3(const mat3 &mat, const vec3f &vec);
    }
} // namespace echo

inline echo::mat4 operator*(const echo::mat4 &a, const echo::mat4 &b)
{
    return echo::_priv::multiply(a, b);
}

inline echo::vec4f operator*(const echo::mat4 &a, const echo::vec4f &b)
{
    return echo::_priv::multiply(a, b);
}

inline const echo::vec3f operator*(const echo::mat4 &a, const echo::vec3f &b)
{
    return echo::_priv::multiplyVec3(a, b);
}

inline const echo::vec3f operator*(const echo::mat3 &a, const echo::vec3f &b)
{
    return echo::_priv::multiplyVec3(a, b);
}