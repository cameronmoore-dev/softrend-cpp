#pragma once

#include <vector>
#include <stdint.h>

#include "Structs.h"
#include "Utils.h"
#include "Matrix.h"
#include "Image.h"

class Renderer
{
    friend class Window;

private:
    Vec3f m_bbMin;
    Vec3f m_bbMax;

    Vec4f clipSpace = {};
    float oneOverZ = 0;

private:
    void calculateClipSpace(const Vec3f& vertexPos, const Mat4& model, const Mat4& view, const Mat4& projection);
    void perspectiveDivision();
    void calculateScreenSpace(Vec3f& vertexPos, const Vec2i& windowSize);

    void boundingBox(const Vec3f& a, const Vec3f& b, const Vec3f& c);
    Vec3f barycentric(const Vec2i& p, const Vec3f& a, const Vec3f& b, const Vec3f& c);
    uint32_t texel(const Vertex& v0, const Vertex& v1, const Vertex& v2, const Image& image, const Vec3f& weights);
    void rasterise(std::vector<uint32_t>& backBuffer, std::vector<float>& depthBuffer, float intensity, const Vertex& v0, const Vertex& v1, const Vertex& v2, uint32_t width, uint32_t height, const Image& image);
};