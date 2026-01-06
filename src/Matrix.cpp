#include "Matrix.h"
#include "math.h"

#include <cmath>
#include <iostream>

Mat4 Matrix::multiply(const Mat4& matA, const Mat4& matB)
{
    uint32_t ar = matA.size();
    uint32_t ac = matA[0].size();
    uint32_t bc = matB[0].size();

    Mat4 result = {};
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
    Vec4f vec = toVec4(_vec, 1.f);

    Vec4f result = {};
    result.x = mat[0][0] * vec.x + mat[0][1] * vec.y + mat[0][2] * vec.z + mat[0][3] * vec.w;
    result.y = mat[1][0] * vec.x + mat[1][1] * vec.y + mat[1][2] * vec.z + mat[1][3] * vec.w;
    result.z = mat[2][0] * vec.x + mat[2][1] * vec.y + mat[2][2] * vec.z + mat[2][3] * vec.w;
    result.w = mat[3][0] * vec.x + mat[3][1] * vec.y + mat[3][2] * vec.z + mat[3][3] * vec.w;

    return result;
}

Vec4f Matrix::multiply(const Mat4& mat, const Vec4f& vec)
{
    Vec4f result = {};
    result.x = mat[0][0] * vec.x + mat[0][1] * vec.y + mat[0][2] * vec.z + mat[0][3] * vec.w;
    result.y = mat[1][0] * vec.x + mat[1][1] * vec.y + mat[1][2] * vec.z + mat[1][3] * vec.w;
    result.z = mat[2][0] * vec.x + mat[2][1] * vec.y + mat[2][2] * vec.z + mat[2][3] * vec.w;
    result.w = mat[3][0] * vec.x + mat[3][1] * vec.y + mat[3][2] * vec.z + mat[3][3] * vec.w;
    return result;
}

Vec3f Matrix::multiplyVec3(const Mat4& mat, const Vec3f& vec)
{
    Vec3f result = {};
    result.x = mat[0][0] * vec.x + mat[0][1] * vec.y + mat[0][2] * vec.z + mat[0][3];
    result.y = mat[1][0] * vec.x + mat[1][1] * vec.y + mat[1][2] * vec.z + mat[1][3];
    result.z = mat[2][0] * vec.x + mat[2][1] * vec.y + mat[2][2] * vec.z + mat[2][3];
    return result;
}

Vec3f Matrix::multiplyVec3(const Mat3& mat, const Vec3f& vec)
{
    Vec3f result = {};
    result.x = mat[0][0] * vec.x + mat[0][1] * vec.y + mat[0][2] * vec.z;
    result.y = mat[1][0] * vec.x + mat[1][1] * vec.y + mat[1][2] * vec.z;
    result.z = mat[2][0] * vec.x + mat[2][1] * vec.y + mat[2][2] * vec.z;
    return result;
}

Mat3 Matrix::constructTBN(const Vec3f& t, const Vec3f& b, const Vec3f& n)
{
    Mat3 tbn;
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
    result[1][1] = 2.f / (top - bottom);
    result[1][3] = -(top + bottom) / (top - bottom);
    result[2][2] = -2.f / (zFar - zNear);
    result[2][3] = -(zFar + zNear) / (zFar - zNear);
    result[3][3] = 1.f;

    return result;
}

Mat4 Matrix::lookAt(const Vec3f& pos, const Vec3f& forward, const Vec3f& up)
{
    const Vec3f f = math::normalise(pos - forward);        /* Local Forward */
    const Vec3f s = math::normalise(math::cross(f, up));   /* Local Side */
    const Vec3f u = math::cross(s, f);                     /* Local Up */

    Mat4 result = {};
    result[0][0] = s.x;
    result[0][1] = s.y;
    result[0][2] = s.z;
    result[0][3] = -math::dot(s, pos);

    result[1][0] = u.x;
    result[1][1] = u.y;
    result[1][2] = u.z;
    result[1][3] = -math::dot(u, pos);

    result[2][0] = -f.x;
    result[2][1] = -f.y;
    result[2][2] = -f.z;
    result[2][3] = math::dot(f, pos);

    result[3][3] = 1.f;

    return result;
}

Mat4 Matrix::translate(const Vec3f& vec)
{
    Mat4 t = identity();

    t[0][3] = vec.x;
    t[1][3] = vec.y;
    t[2][3] = vec.z;

    return t;
}

Mat4 Matrix::rotate(const float angle, const Vec3f &axis)
{
    const float _angle = math::degToRad(angle);
    const Vec3f _axis = math::normalise(axis);

    float c = cos(angle);
    float s = sin(angle);
    float t = 1 - c;

    Mat4 r = identity();

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

Mat4 Matrix::scale(const Vec3f& vec)
{
    Mat4 s = identity();

    s[0][0] = vec.x;
    s[1][1] = vec.y;
    s[2][2] = vec.z;
    s[3][3] = 1.f;

    return s;
}

Mat4 Matrix::transform(const Mat4 &t, const Mat4 &r, const Mat4 &s)
{
    return multiply(t, multiply(r, s));
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

const Vec4f Matrix::toVec4(const Vec3f& vec, float w)
{
    return { vec.x, vec.y, vec.z, w };
}