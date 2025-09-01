#include "Renderer.h"

#include <algorithm>

void Renderer::calculateClipSpace(const Vec3f& vertexPos, const Mat4& model, const Mat4& view, const Mat4& projection)
{
    Mat4 mv = Matrix::multiply(view, model);
    Mat4 mvp = Matrix::multiply(projection, mv);
    clipSpace = Matrix::multiply(mvp, vertexPos);

    oneOverZ = 1/clipSpace.w;
}

void Renderer::perspectiveDivision()
{
    if(clipSpace.w != 0)
    {
        clipSpace.x /= clipSpace.w;
        clipSpace.y /= clipSpace.w;
        clipSpace.z /= clipSpace.w;
    }
}

void Renderer::calculateScreenSpace(Vec3f& vertexPos, const Vec2i& windowSize)
{
    vertexPos.x = (clipSpace.x + 1) * 0.5f * windowSize.x;
    vertexPos.y = (1 - clipSpace.y) * 0.5f * windowSize.y;
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

void Renderer::rasterise(std::vector<uint32_t>& backBuffer, std::vector<float>& depthBuffer, float intensity, const Vertex& v0, const Vertex& v1, const Vertex& v2, uint32_t width, uint32_t height, const Image& image)
{
    /* Back-face Culling */
    Vec3f normals = Utils::getNormals(v0.position, v1.position, v2.position);
    if (-normals.z <= 0.f)
    {
        return;
    }

    boundingBox(v0.position, v1.position, v2.position);

    Vec2i p;
    for (p.y = m_bbMin.y; p.y <= m_bbMax.y; p.y++)
    {
        for (p.x = m_bbMin.x; p.x <= m_bbMax.x; p.x++)
        {
            if (p.x < 0 || p.y < 0 || p.x >= width || p.y >= height)
            {
                continue;
            }

            Vec3f point = barycentric(p, v0.position, v1.position, v2.position);
            if (point.x >= 0 && point.y >= 0 && point.z >= 0)
            {
                float z = v0.position.z * point.x + v1.position.z * point.y + v2.position.z * point.z;
                uint32_t idx = p.x + p.y * width;
                if (z < depthBuffer[idx])
                {
                    depthBuffer[idx] = z;
                    backBuffer[idx] = texel(v0, v1, v2, image, point);
                }
            }
        }
    }
}

UINT Renderer::texel(const Vertex& v0, const Vertex& v1, const Vertex& v2, const Image& image, const Vec3f& weights)
{
    Vec2f v0OverZ = v0.texCoords * oneOverZ;
    Vec2f v1OverZ = v1.texCoords * oneOverZ;
    Vec2f v2OverZ = v2.texCoords * oneOverZ;

    /* Perspective-correct interpolation */
    float invInterp = weights.x * oneOverZ +
                      weights.y * oneOverZ +
                      weights.z * oneOverZ;

    Vec2f uvOverZInterp = v0OverZ * weights.x +
    v1OverZ * weights.y +
    v2OverZ * weights.z;

    Vec2f finalUV = uvOverZInterp / invInterp;

    int32_t texX = finalUV.x * (image.size().x - 1);
    int32_t texY = finalUV.y * (image.size().y - 1);

    texX = std::clamp(texX, 0, image.size().x - 1);
    texY = std::clamp(texY, 0, image.size().y - 1);

    return image.pixel(texX + texY * image.size().x);
}