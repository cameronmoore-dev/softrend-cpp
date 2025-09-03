#pragma once

#include "Structs.h"

#include <array>
#include <stdint.h>

using Mat4 = std::array<std::array<float, 4>, 4>;

class Matrix
{
public:
    static Mat4 multiply(const Mat4& matA, const Mat4& matB);
    static Vec4f multiply(const Mat4& mat, const Vec3f& vec);
    static Vec4f multiply(const Mat4& mat, const Vec4f& vec);

    static Mat4 perspective(float fov, float aspect, float zNear, float zFar);
    static Mat4 orthographic(int32_t left, int32_t right, int32_t bottom, int32_t top, float zNear, float zFar);
    static Mat4 lookAt(const Vec3f& pos, const Vec3f& forward, const Vec3f& up);

    static Mat4 translate(const Vec3f& vec);
    static Mat4 rotate(float angle, uint32_t axis);
    static Mat4 scale(const Vec3f& vec);

    static Mat4 identity();
};