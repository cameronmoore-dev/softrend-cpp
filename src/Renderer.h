#pragma once

#include <vector>
#include <stdint.h>

#include "Window.h"
#include "Structs.h"
#include "Utils.h"
#include "Matrix.h"
#include "Image.h"
#include "Camera.h"

class Renderer
{
private:
    struct Triangle
    {
        Vertex v0, v1, v2;
    };

    struct RasterVertex
    {
        Vec3f screenPos;
        Vec2f uvOverZ;
        float inverseZ;
        uint32_t colour;
    };

public:
    Renderer(Window& window);
    void draw(const std::vector<Vertex>& vertices, Image* texture, Camera& camera, const Mat4& model);

private:
    Window& m_context;

    Vec2f m_bbMin;
    Vec2f m_bbMax;

private:
    Triangle toModelSpace(const Vertex& v0, const Vertex& v1, const Vertex& v2, const Mat4& model);
    std::vector<Vertex> toViewSpace(const Triangle& tri, const Camera& camera);
    RasterVertex toScreenSpace(const Vertex& vertex, const Mat4& projection);
    
    void rasterise(const RasterVertex& v0, const RasterVertex& v1, const RasterVertex& v2, Image* image);
    void boundingBox(const Vec3f& a, const Vec3f& b, const Vec3f& c);
    Vec3f barycentric(const Vec2i& p, const Vec3f& a, const Vec3f& b, const Vec3f& c);
    float edgeFunction(const Vec3f& a, const Vec3f& b, const Vec3f& c);
    uint32_t texel(const RasterVertex& v0, const RasterVertex& v1, const RasterVertex& v2, const Image& image, const Vec3f& weights);

    std::vector<Vertex> nearPlaneClip(const Vertex& a, const Vertex& b, const Vertex& c, float zNear, float zFar);
    Vertex interpolate(const Vertex& a, const Vertex& b, float t);

    
};