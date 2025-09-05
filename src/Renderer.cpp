#include "Renderer.h"

#include <algorithm>
#include <iostream>

Renderer::Triangle Renderer::modelSpace(const Vertex& v0, const Vertex& v1, const Vertex& v2, const Mat4& model)
{
    Renderer::Triangle triangle{ v0, v1, v2 };

    triangle.v0.position = Utils::toVec3(Matrix::multiply(model, v0.position));
    triangle.v1.position = Utils::toVec3(Matrix::multiply(model, v1.position));
    triangle.v2.position = Utils::toVec3(Matrix::multiply(model, v2.position));

    return triangle;
}

std::vector<Renderer::Triangle> Renderer::viewSpace(const Renderer::Triangle& tri, const Mat4& view)
{
    Renderer::Triangle transformed = tri;
    transformed.v0.position = Utils::toVec3(Matrix::multiply(view, tri.v0.position));
    transformed.v1.position = Utils::toVec3(Matrix::multiply(view, tri.v1.position));
    transformed.v2.position = Utils::toVec3(Matrix::multiply(view, tri.v2.position));

    std::vector<Renderer::Triangle> clippedTris;
    std::vector<Vertex> clipped;
    int triCount = nearPlaneClip(clipped, transformed.v0, transformed.v1, transformed.v2, 0.5f);

    for (uint8_t i = 0; i < triCount; i++)
    {
        clippedTris.emplace_back(Renderer::Triangle{
            clipped[i * 3 + 0],
            clipped[i * 3 + 1],
            clipped[i * 3 + 2]
        });
    }

    return clippedTris;
}

bool Renderer::calculateScreenSpace(Renderer::RasterVertex& rv, const Vertex& vertex, const Mat4& projection, const uint32_t width, const uint32_t height)
{
    Vec4f clipSpace = Matrix::multiply(projection, vertex.position);

    clipSpace.x /= clipSpace.w;
    clipSpace.y /= clipSpace.w;
    clipSpace.z /= clipSpace.w;

    Vec3f screenPos;
    screenPos.x = (clipSpace.x + 1) * 0.5f * width;
    screenPos.y = (clipSpace.y + 1) * 0.5f * height;
    screenPos.z = clipSpace.z;

    rv.screenPos = screenPos;
    rv.inverseZ = 1.f / clipSpace.w;
    rv.uvOverZ = vertex.texCoords * rv.inverseZ;
    rv.colour = vertex.colour;
    return true;
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

Vertex Renderer::interpolate(const Vertex& a, const Vertex& b, float t)
{
    Vertex out;
    out.position = a.position + (b.position - a.position) * t;
    out.texCoords = a.texCoords + (b.texCoords - a.texCoords) * t;

    return out;
}

int Renderer::nearPlaneClip(std::vector<Vertex>& out, const Vertex& a, const Vertex& b, const Vertex& c, float zNear)
{
    const Vertex in[3] = { a, b, c };
    bool inside[3] = { a.position.z <= -zNear, b.position.z <= -zNear, c.position.z <= -zNear };

    int inCount = inside[0] + inside[1] + inside[2];

    if(inCount == 0) return 0;

    if (inCount == 3)
    {
        out.emplace_back(a);
        out.emplace_back(b);
        out.emplace_back(c);
        return 1;
    }

    Vertex inV[2];
    Vertex outV;
    if (inCount == 1)
    {
        int inIdx = inside[0] ? 0 : inside[1] ? 1 : 2;
        int outIdx1 = (inIdx + 1) % 3;
        int outIdx2 = (inIdx + 2) % 3;

        const Vertex& A = in[inIdx];
        const Vertex& B = in[outIdx1];
        const Vertex& C = in[outIdx2];

        float tAB = (-zNear - A.position.z) / (B.position.z - A.position.z);
        float tAC = (-zNear - A.position.z) / (C.position.z - A.position.z);

        Vertex clipped1 = interpolate(A, B, tAB);
        Vertex clipped2 = interpolate(A, C, tAC);

        out.emplace_back(A);
        out.emplace_back(clipped1);
        out.emplace_back(clipped2);
        return 1;
    }

    if (inCount == 2)
    {
        int outIdx = !inside[0] ? 0 : (!inside[1] ? 1 : 2);
        int inIdx1 = (outIdx + 1) % 3;
        int inIdx2 = (outIdx + 2) % 3;

        const Vertex& A = in[inIdx1];
        const Vertex& B = in[inIdx2];
        const Vertex& C = in[outIdx];

        float tAC = (-zNear - A.position.z) / (C.position.z - A.position.z);
        float tBC = (-zNear - B.position.z) / (C.position.z - B.position.z);

        Vertex clippedA = interpolate(A, C, tAC);
        Vertex clippedB = interpolate(B, C, tBC);

        out.emplace_back(A);
        out.emplace_back(B);
        out.emplace_back(clippedA);

        out.emplace_back(B);
        out.emplace_back(clippedB);
        out.emplace_back(clippedA);
        return 2;
    }

    return 0;
}

std::array<Renderer::Plane, 6> Renderer::extractFrustrumPlanes(const Mat4& vp)
{
    std::array<Plane, 6> planes;

    /* Left */
    planes[0].normal.x = vp[0][3] + vp[0][0];
    planes[0].normal.y = vp[1][3] + vp[1][0];
    planes[0].normal.z = vp[2][3] + vp[2][0];
    planes[0].distance = vp[3][3] + vp[3][0];

    // Right
    planes[1].normal.x = vp[0][3] - vp[0][0];
    planes[1].normal.y = vp[1][3] - vp[1][0];
    planes[1].normal.z = vp[2][3] - vp[2][0];
    planes[1].distance = vp[3][3] - vp[3][0];

    // Bottom
    planes[2].normal.x = vp[0][3] + vp[0][1];
    planes[2].normal.y = vp[1][3] + vp[1][1];
    planes[2].normal.z = vp[2][3] + vp[2][1];
    planes[2].distance = vp[3][3] + vp[3][1];

    // Top
    planes[3].normal.x = vp[0][3] - vp[0][1];
    planes[3].normal.y = vp[1][3] - vp[1][1];
    planes[3].normal.z = vp[2][3] - vp[2][1];
    planes[3].distance = vp[3][3] - vp[3][1];

    // Near
    planes[4].normal.x = vp[0][3] + vp[0][2];
    planes[4].normal.y = vp[1][3] + vp[1][2];
    planes[4].normal.z = vp[2][3] + vp[2][2];
    planes[4].distance = vp[3][3] + vp[3][2];

    // Far
    planes[5].normal.x = vp[0][3] - vp[0][2];
    planes[5].normal.y = vp[1][3] - vp[1][2];
    planes[5].normal.z = vp[2][3] - vp[2][2];
    planes[5].distance = vp[3][3] - vp[3][2];

    for (Plane& p : planes)
    {
        float length = Utils::length(p.normal);
        p.normal /= length;
        p.distance /= length;
    }

    return planes;
}

bool Renderer::isSphereInsideFrustrum(const Vec3f& centre, float radius, const std::array<Plane, 6>& planes)
{
    for (const Plane& p : planes)
    {
        if (p.distanceToPoint(centre) < -radius)
        {
            return false;
        }
    }

    return true;
}