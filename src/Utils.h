#pragma once

#include <minwindef.h>

#include "Structs.h"
#include "Matrix.h"

class Utils
{
public:
    static const UINT lerpHex(const UINT a, const UINT b, const FLOAT t);
    static const UINT rasteriseColour(const UINT c0, const UINT c1, const UINT c2, const FLOAT w0, const FLOAT w1, const FLOAT w2);
    static const UINT pcgHash(const UINT seed);
    static const UINT applyLighting(const UINT baseColour, const FLOAT intensity);

    static const Vec3f getViewSpaceNormals(const Mat4& mv, const Vec3f& a, const Vec3f& b, const Vec3f& c);
    static const Vec3f getNormals(const Vec3f& a, const Vec3f& b, const Vec3f& c);
    static const Vec3f normalise(const Vec3f& vec);
    static const Vec3f cross(const Vec3f& a, const Vec3f& b);
    static const FLOAT dot(const Vec3f& a, const Vec3f& b);

    static const FLOAT degToRad(const FLOAT& deg);
    static const Vec4f toVec4(const Vec3f& vec, FLOAT w = 1.f);
    static const Vec3f toVec3(const Vec4f& vec);
    static const Vec3f screenToClip(const Vec3f& pos, INT width, INT height);

    static const FLOAT lerp(const FLOAT& a, const FLOAT& b, FLOAT t);

    static const void drawLine(const Vertex& v1, const Vertex& v2);

public:
    inline static const FLOAT pi = 3.14159265359f;

private:
    static const void drawLineLow(const Vertex& v1, const Vertex& v2);
    static const void drawLineHigh(const Vertex& v1, const Vertex& v2);
};