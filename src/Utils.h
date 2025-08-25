#pragma once

#include <minwindef.h>

#include "Structs.h"

class Utils
{
public:
    static UINT lerpHex(UINT a, UINT b, FLOAT t);
    static UINT rasteriseColour(UINT c0, UINT c1, UINT c2, FLOAT w0, FLOAT w1, FLOAT w2);
    static UINT pcgHash(UINT seed);

    static Vec3f cross(const Vec3f& a, const Vec3f& b);
    static FLOAT dot(const Vec3i& a, const Vec3i& b);
    static FLOAT degToRad(const FLOAT& deg);
    static Vec4f toVec4(const Vec3f& vec, FLOAT w = 1.f);
    static Vec3f screenToClip(const Vec3f& pos, INT width, INT height);

    static void drawLine(const Vertex& v1, const Vertex& v2);

public:
    inline static const FLOAT pi = 3.14159265359f;

private:
    static void drawLineLow(const Vertex& v1, const Vertex& v2);
    static void drawLineHigh(const Vertex& v1, const Vertex& v2);
};