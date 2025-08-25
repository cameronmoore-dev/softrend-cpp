#pragma once

#include "Structs.h"

#include <array>
#include <minwindef.h>

using Mat4 = std::array<std::array<FLOAT, 4>, 4>;
using Mat3 = std::array<std::array<FLOAT, 3>, 3>;

using Mat3x1 = std::array<std::array<FLOAT, 3>, 1>;
using Mat4x1 = std::array<std::array<FLOAT, 4>, 1>;

class Matrix
{
public:
    static Mat4 multiply(const Mat4& matA, const Mat4& matB);
    static Vec4f multiply(const Mat4& mat, const Vec3f& vec);

    static Mat4 perspective(FLOAT fov, FLOAT aspect, FLOAT zNear, FLOAT zFar);
    static Mat4 orthographic(INT left, INT right, INT bottom, INT top, FLOAT zNear, FLOAT zFar);
    static Mat4 lookAt();

    static Mat4 translate(const Vec3f& vec);
    static Mat4 rotate(FLOAT angle, UINT axis);
    static Mat4 scale(const Vec3f& vec);

    static Mat4 identity();
};