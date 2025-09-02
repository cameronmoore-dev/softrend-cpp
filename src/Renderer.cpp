#include "Renderer.h"

#include <algorithm>

Renderer::RasterVertex Renderer::calculateScreenSpace(const Vertex& vertex, const Mat4& model, const Mat4& view, const Mat4& projection, const uint32_t width, const uint32_t height)
{
    RasterVertex rv;

    Vec4f m = Matrix::multiply(model, vertex.position);
    Vec4f mv = Matrix::multiply(view, m);
    Vec4f clipSpace = Matrix::multiply(projection, mv);
    
    if (clipSpace.w != 0)
    {
        clipSpace.x /= clipSpace.w;
        clipSpace.y /= clipSpace.w;
        clipSpace.z /= clipSpace.w;
    }

    Vec3f screenPos;
    screenPos.x = (clipSpace.x + 1) * 0.5f * width;
    screenPos.y = (1 + clipSpace.y) * 0.5f * height;
    screenPos.z = clipSpace.z;

    rv.screenPos = screenPos;
    rv.inverseZ = 1.f / clipSpace.w;
    rv.uvOverZ = vertex.texCoords * rv.inverseZ;
    rv.colour = vertex.colour;
    return rv;
}

void Renderer::rasterise(std::vector<uint32_t>& backBuffer, std::vector<float>& depthBuffer, float intensity, const RasterVertex& v0, const RasterVertex& v1, const RasterVertex& v2, uint32_t width, uint32_t height, Image* image)
{
    /* Back-face Culling */
    float winding = edgeFunction(v0.screenPos, v1.screenPos, v2.screenPos);
    if (winding <= 0)
    {
        return;
    }

    boundingBox(v0.screenPos, v1.screenPos, v2.screenPos);

    Vec2i p;
    for (p.y = m_bbMin.y; p.y <= m_bbMax.y; p.y++)
    {
        for (p.x = m_bbMin.x; p.x <= m_bbMax.x; p.x++)
        {
            if (p.x < 0 || p.y < 0 || p.x >= width || p.y >= height)
            {
                continue;
            }

            Vec3f point = barycentric(p, v0.screenPos, v1.screenPos, v2.screenPos);
            if (point.x >= 0 && point.y >= 0 && point.z >= 0)
            {
                float z = v0.screenPos.z * point.x + v1.screenPos.z * point.y + v2.screenPos.z * point.z;
                uint32_t idx = p.x + p.y * width;
                if (z < depthBuffer[idx])
                {
                    depthBuffer[idx] = z;

                    if (image)
                    {
                        backBuffer[idx] = texel(v0, v1, v2, *image, point);
                    }
                    else
                    {
                        backBuffer[idx] = Utils::rasteriseColour(v0.colour, v1.colour, v2.colour, point.x, point.y, point.z);
                    }
                }
            }
        }
    }
}

void Renderer::boundingBox(const Vec3f& a, const Vec3f& b, const Vec3f& c)
{
    float leftMost   = std::min(a.x, std::min(b.x, c.x));
    float bottomMost = std::min(a.y, std::min(b.y, c.y));
    float rightMost  = std::max(a.x, std::max(b.x, c.x));
    float topMost    = std::max(a.y, std::max(b.y, c.y));

    m_bbMin = { leftMost, bottomMost, 0 };
    m_bbMax = { rightMost, topMost, 0 };
}

Vec3f Renderer::barycentric(const Vec2i& p, const Vec3f& a, const Vec3f& b, const Vec3f& c)
{
    float denom = ((b.y-c.y)*(a.x-c.x)+(c.x-b.x)*(a.y-c.y));
    float w1 = ((b.y-c.y)*(p.x-c.x)+(c.x-b.x)*(p.y-c.y)) / denom;
    float w2 = ((c.y-a.y)*(p.x-c.x)+(a.x-c.x)*(p.y-c.y)) / denom;
    float w3 = 1.0f - w1 - w2;

    return { w1, w2, w3 };
}

uint32_t Renderer::texel(const RasterVertex& v0, const RasterVertex& v1, const RasterVertex& v2, const Image& image, const Vec3f& weights)
{

    /* Perspective-correct interpolation */

    float invInterp = v0.inverseZ * weights.x +
                      v1.inverseZ * weights.y +
                      v2.inverseZ * weights.z;

    Vec2f uvOverZInterp = v0.uvOverZ * weights.x +
                          v1.uvOverZ * weights.y +
                          v2.uvOverZ * weights.z;

    Vec2f finalUV = uvOverZInterp / invInterp;

    int32_t texX = finalUV.x * (image.size().x - 1);
    int32_t texY = finalUV.y * (image.size().y - 1);

    texX = std::clamp(texX, 0, image.size().x - 1);
    texY = std::clamp(texY, 0, image.size().y - 1);

    return image.pixel(texX + texY * image.size().x);
}

float Renderer::edgeFunction(const Vec3f& a, const Vec3f& b, const Vec3f& c)
{
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}