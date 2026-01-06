#include "math.h"

namespace echo
{
    const float lerp(const float a, const float b, float t)
    {
        return a + t * (b-a);
    }

    const float degToRad(const float deg)
    {
        return deg * (pi / 180.f);
    }

    const float dot(const vec3f &a, const vec3f &b)
    {
        return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
    }

    const float length(const vec3f &normal)
    {
        return sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
    }

    const vec3f normalise(const vec3f &vec)
    {
        float mag = sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
        return { vec.x / mag, vec.y / mag, vec.z / mag };
    }

    const vec3f cross(const vec3f &a, const vec3f &b)
    {
        vec3f c;
        c.x = a.y * b.z - a.z * b.y;
        c.y = a.z * b.x - a.x * b.z;
        c.z = a.x * b.y - a.y * b.x;

        return c;
    }

    const vec4f srgbToLinear(const vec4f &colour)
    {
        vec4f linear;
        linear.x = std::powf(colour.x, 2.2f);
        linear.y = std::powf(colour.y, 2.2f);
        linear.z = std::powf(colour.z, 2.2f);
        linear.w = std::powf(colour.w, 2.2f);
        
        return linear;
    }

    const vec4f linearToSRGB(const vec4f &colour)
    {
        vec4f srgb;
        srgb.x = std::powf(colour.x, 1.f / 2.2f);
        srgb.y = std::powf(colour.y, 1.f / 2.2f);
        srgb.z = std::powf(colour.z, 1.f / 2.2f);
        srgb.w = std::powf(colour.w, 1.f / 2.2f);

        return srgb;
    }

    const uint32_t rgbToHex(const vec3f &colour) 
    {
        int R = std::clamp(int(colour.x * 255.f), 0, 255);
        int G = std::clamp(int(colour.y * 255.f), 0, 255);
        int B = std::clamp(int(colour.z * 255.f), 0, 255);
        return (R << 16) | (G << 8) | B;
    }

    const vec3f hexToRGB(const uint32_t colour)
    {
        float r = float((colour >> 16) & 0xFF) / 255.f;
        float g = float((colour >> 8) & 0xFF) / 255.f;
        float b = float(colour & 0xFF) / 255.f;
        return  { r, g, b };
    }

    vec3f vec3(const vec4f &vec)
    {
        return { vec.x, vec.y, vec.z };
    }

    vec4f vec4(const vec3f &vec, const float w)
    {
        return { vec.x, vec.y, vec.z, w };
    }

    vec4f vec4(const vec2f &vec, const float z, const float w)
    {
        return { vec.x, vec.y, z, w };
    }

    const mat3 constructTBN(const vec3f &t, const vec3f &b, const vec3f &n)
    {
        mat3 tbn;
        tbn[0][0] = t.x;
        tbn[1][0] = t.y;
        tbn[2][0] = t.z;

        tbn[0][1] = b.x;
        tbn[1][1] = b.y;
        tbn[2][1] = b.z;

        tbn[0][2] = n.x;
        tbn[1][2] = n.y;
        tbn[2][2] = n.z;

        return tbn;
    }

    const mat4 perspective(const float fov, const float aspect, const float zNear, const float zFar)
    {
        mat4 result = {};

        float f = 1.f / std::tan(fov * 0.5f);
        result[0][0] = f / aspect;
        result[1][1] = f;
        result[2][2] = -(zFar + zNear) / (zFar - zNear);
        result[2][3] = -(2.f * zFar * zNear) / (zFar - zNear);
        result[3][2] = -1.f;

        return result;
    }

    const mat4 orthographic(const int32_t left, const int32_t right, const int32_t bottom, const int32_t top, const float zNear, const float zFar)
    {
        mat4 result = {};
        result[0][0] = 2.f / (right - left);
        result[0][3] = -(right + left) / (right - left);
        result[1][1] = 2.f / (top - bottom);
        result[1][3] = -(top + bottom) / (top - bottom);
        result[2][2] = -2.f / (zFar - zNear);
        result[2][3] = -(zFar + zNear) / (zFar - zNear);
        result[3][3] = 1.f;

        return result;
    }

    const mat4 lookAt(const vec3f &pos, const vec3f &forward, const vec3f &up)
    {
        const vec3f f = normalise(pos - forward);        /* Local Forward */
        const vec3f s = normalise(cross(f, up));   /* Local Side */
        const vec3f u = cross(s, f);                     /* Local Up */

        mat4 result = {};
        result[0][0] = s.x;
        result[0][1] = s.y;
        result[0][2] = s.z;
        result[0][3] = -dot(s, pos);

        result[1][0] = u.x;
        result[1][1] = u.y;
        result[1][2] = u.z;
        result[1][3] = -dot(u, pos);

        result[2][0] = -f.x;
        result[2][1] = -f.y;
        result[2][2] = -f.z;
        result[2][3] = dot(f, pos);

        result[3][3] = 1.f;

        return result;
    }

    const mat4 translate(const vec3f &vec)
    {
        mat4 t = identity();

        t[0][3] = vec.x;
        t[1][3] = vec.y;
        t[2][3] = vec.z;

        return t;
    }

    const mat4 rotate(const float angle, const vec3f &axis)
    {
        const float _angle = degToRad(angle);
        const vec3f _axis = normalise(axis);

        float c = cos(angle);
        float s = sin(angle);
        float t = 1 - c;

        mat4 r = identity();

        r[0][0] = t * _axis.x * _axis.x + c;
        r[0][1] = t * _axis.x * _axis.y - s * _axis.z;
        r[0][2] = t * _axis.x * _axis.z + s * _axis.y;

        r[1][0] = t * _axis.x * _axis.y + s * _axis.z;
        r[1][1] = t * _axis.y * _axis.y + c;
        r[1][2] = t * _axis.y * _axis.z - s * _axis.x;

        r[2][0] = t * _axis.x * _axis.z - s * _axis.y;
        r[2][1] = t * _axis.y * _axis.z + s * _axis.x;
        r[2][2] = t * _axis.z * _axis.z + c;

        return r;
    }

    const mat4 scale(const vec3f &vec)
    {
        mat4 s = identity();

        s[0][0] = vec.x;
        s[1][1] = vec.y;
        s[2][2] = vec.z;
        s[3][3] = 1.f;

        return s;
    }

    const mat4 identity()
    {
        mat4 result = {};
        result[0][0] = 1.f;
        result[1][1] = 1.f;
        result[2][2] = 1.f;
        result[3][3] = 1.f;

        return result;
    }

    namespace _priv
    {
        const mat4 multiply(const mat4 &matA, const mat4 &matB)
        {
            uint32_t ar = matA.size();
            uint32_t ac = matA[0].size();
            uint32_t bc = matB[0].size();

            mat4 result = {};
            for (uint32_t row = 0; row < ar; row++)
            {
                for (uint32_t col = 0; col < bc; col++)
                {
                    for (uint32_t e = 0; e < ac; e++)
                    {
                        result[row][col] += matA[row][e] * matB[e][col];
                    }
                }
            }
            return result;
        }

        const vec4f multiply(const mat4 &mat, const vec4f &vec)
        {
            vec4f result = {};
            result.x = mat[0][0] * vec.x + mat[0][1] * vec.y + mat[0][2] * vec.z + mat[0][3] * vec.w;
            result.y = mat[1][0] * vec.x + mat[1][1] * vec.y + mat[1][2] * vec.z + mat[1][3] * vec.w;
            result.z = mat[2][0] * vec.x + mat[2][1] * vec.y + mat[2][2] * vec.z + mat[2][3] * vec.w;
            result.w = mat[3][0] * vec.x + mat[3][1] * vec.y + mat[3][2] * vec.z + mat[3][3] * vec.w;

            return result;
        }

        const vec3f multiplyVec3(const mat4 &mat, const vec3f &vec)
        {
            vec3f result = {};
            result.x = mat[0][0] * vec.x + mat[0][1] * vec.y + mat[0][2] * vec.z + mat[0][3];
            result.y = mat[1][0] * vec.x + mat[1][1] * vec.y + mat[1][2] * vec.z + mat[1][3];
            result.z = mat[2][0] * vec.x + mat[2][1] * vec.y + mat[2][2] * vec.z + mat[2][3];

            return result;
        }

        const vec3f multiplyVec3(const mat3 &mat, const vec3f &vec)
        {
            vec3f result = {};
            result.x = mat[0][0] * vec.x + mat[0][1] * vec.y + mat[0][2] * vec.z;
            result.y = mat[1][0] * vec.x + mat[1][1] * vec.y + mat[1][2] * vec.z;
            result.z = mat[2][0] * vec.x + mat[2][1] * vec.y + mat[2][2] * vec.z;

            return result;
        }
    }
}