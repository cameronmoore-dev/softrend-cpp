#include "Matrix.h"
#include "Utils.h"

#include <cmath>
#include <iostream>

Mat4 Matrix::multiply(const Mat4& matA, const Mat4& matB)
{
    uint32_t ar = matA.size();
    uint32_t ac = matA[0].size();

    uint32_t bc = matB[0].size();

    Mat4 result= {};
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

Vec4f Matrix::multiply(const Mat4& mat, const Vec3f& _vec)
{
    Vec4f vec = Utils::toVec4(_vec);

    Vec4f result = {};
    result.x = vec.x * mat[0][0] + vec.y * mat[0][1] + vec.z * mat[0][2] + mat[0][3];
    result.y = vec.x * mat[1][0] + vec.y * mat[1][1] + vec.z * mat[1][2] + mat[1][3];
    result.z = vec.x * mat[2][0] + vec.y * mat[2][1] + vec.z * mat[2][2] + mat[2][3];
    result.w = vec.x * mat[3][0] + vec.y * mat[3][1] + vec.z * mat[3][2] + mat[3][3];

    return result;
}

Vec4f Matrix::multiply(const Mat4& mat, const Vec4f& vec)
{
    Vec4f result = {};
    result.x = vec.x * mat[0][0] + vec.y * mat[0][1] + vec.z * mat[0][2] + mat[0][3];
    result.y = vec.x * mat[1][0] + vec.y * mat[1][1] + vec.z * mat[1][2] + mat[1][3];
    result.z = vec.x * mat[2][0] + vec.y * mat[2][1] + vec.z * mat[2][2] + mat[2][3];
    result.w = vec.x * mat[3][0] + vec.y * mat[3][1] + vec.z * mat[3][2] + mat[3][3];

    return result;
}

Mat4 Matrix::perspective(float fov, float aspect, float zNear, float zFar)
{
    Mat4 result = {};

    float f = 1.f / std::tan(fov * 0.5f);
    result[0][0] = f / aspect;
    result[1][1] = f;
    result[2][2] = -(zFar + zNear) / (zFar - zNear);
    result[2][3] = -(2.f * zFar * zNear) / (zFar - zNear);
    result[3][2] = -1.f;

    return result;
}

Mat4 Matrix::orthographic(int32_t left, int32_t right, int32_t bottom, int32_t top, float zNear, float zFar)
{
    Mat4 result = {};
    result[0][0] = 2.f / (right - left);
    result[0][3] = -(right + left) / (right - left);
    result[1][1] = -2.f / (top - bottom);
    result[1][3] = -(top + bottom) / (top - bottom);
    result[2][2] = -2.f / (zFar - zNear);
    result[2][3] = -(zFar + zNear) / (zFar - zNear);
    result[3][3] = 1.f;

    return result;
}

Mat4 Matrix::translate(const Vec3f& vec)
{
    Mat4 result = identity();
    result[0][3] = vec.x;
    result[1][3] = vec.y;
    result[2][3] = vec.z;

    return result;
}

Mat4 Matrix::rotate(float angle, uint32_t axis)
{
    Mat4 result = identity();
    
    angle = Utils::degToRad(angle);

    //x
    if(axis == 0)
    {
        result[0][0] = 1.f;
        result[1][1] = std::cos(angle);
        result[1][2] = -std::sin(angle);
        result[2][1] = std::sin(angle);
        result[2][2] = std::cos(angle);
    }

    //y
    else if(axis == 1)
    {
        result[0][0] = std::cos(angle);
        result[0][2] = std::sin(angle);
        result[2][0] = -std::sin(angle);
        result[1][1] = 1;
        result[2][2] = std::cos(angle);
    }

    //z
    else
    {
        result[0][0] = std::cos(angle);
        result[0][1] = -std::sin(angle);
        result[1][0] = std::sin(angle);
        result[1][1] = std::cos(angle);
        result[2][2] = 1;
    }

    return result;
}

Mat4 Matrix::scale(const Vec3f& vec)
{
    Mat4 result = identity();
    result[0][0] = vec.x;
    result[1][1] = vec.y;
    result[2][2] = vec.z;
    result[3][3] = 1.f;

    return result;
}

Mat4 Matrix::identity()
{
    Mat4 result = {};
    result[0][0] = 1.f;
    result[1][1] = 1.f;
    result[2][2] = 1.f;
    result[3][3] = 1.f;

    return result;
}