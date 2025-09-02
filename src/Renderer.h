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
    struct RasterVertex
    {
        Vec3f screenPos;
        Vec2f uvOverZ;
        float inverseZ;
        uint32_t colour;
    };

private:
    Vec3f m_bbMin;
    Vec3f m_bbMax;

    std::vector<RasterVertex> m_rv;

private:
    RasterVertex calculateScreenSpace(const Vertex& vertexPos, const Mat4& model, const Mat4& view, const Mat4& projection, const uint32_t width, const uint32_t height);

    void rasterise(std::vector<uint32_t>& backBuffer, std::vector<float>& depthBuffer, float intensity, const RasterVertex& v0, const RasterVertex& v1, const RasterVertex& v2, uint32_t width, uint32_t height, Image* image);
    void boundingBox(const Vec3f& a, const Vec3f& b, const Vec3f& c);
    Vec3f barycentric(const Vec2i& p, const Vec3f& a, const Vec3f& b, const Vec3f& c);
    uint32_t texel(const RasterVertex& v0, const RasterVertex& v1, const RasterVertex& v2, const Image& image, const Vec3f& weights);

    float edgeFunction(const Vec3f& a, const Vec3f& b, const Vec3f& c);
};