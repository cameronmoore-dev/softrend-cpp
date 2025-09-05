#include "Renderer.h"

#include <algorithm>
#include <iostream>

Renderer::Renderer(Window& window) : 
    m_context(window)
{
}

void Renderer::draw(std::vector<Vertex>& vertices, Image* texture, Camera& camera, const Mat4& model)
{
    Vec3f centre{0, 0, 0};
    float radius = 2.5f;
    if (!camera.isSphereInsideFrustrum(centre, radius))
    {
        return;
    }

    std::vector<Renderer::RasterVertex> rvs;
    for (uint32_t i = 0; i < vertices.size(); i+=3)
    {
        Renderer::Triangle t = modelSpace(vertices[i], vertices[i+1], vertices[i+2], model);
        std::vector<Renderer::Triangle> clippedTris = viewSpace(t, camera.view());
        for (Renderer::Triangle& tr : clippedTris)
        {
            Renderer::RasterVertex rv0, rv1, rv2;
            if (
                calculateScreenSpace(rv0, tr.v0, camera.projection()) &&
                calculateScreenSpace(rv1, tr.v1, camera.projection()) &&
                calculateScreenSpace(rv2, tr.v2, camera.projection()))
            {
                rvs.emplace_back(rv0);
                rvs.emplace_back(rv1);
                rvs.emplace_back(rv2);
            }
        }
    }

    for (uint32_t i = 0; i < rvs.size(); i+=3)
    {
        rasterise(rvs[i], rvs[i+1], rvs[i+2], texture);
    }
}

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

bool Renderer::calculateScreenSpace(Renderer::RasterVertex& rv, const Vertex& vertex, const Mat4& projection)
{
    Vec4f clipSpace = Matrix::multiply(projection, vertex.position);

    clipSpace.x /= clipSpace.w;
    clipSpace.y /= clipSpace.w;
    clipSpace.z /= clipSpace.w;

    Vec3f screenPos;
    screenPos.x = (clipSpace.x + 1) * 0.5f * m_context.m_width;
    screenPos.y = (clipSpace.y + 1) * 0.5f * m_context.m_height;
    screenPos.z = clipSpace.z;

    rv.screenPos = screenPos;
    rv.inverseZ = 1.f / clipSpace.w;
    rv.uvOverZ = vertex.texCoords * rv.inverseZ;
    rv.colour = vertex.colour;
    return true;
}

void Renderer::rasterise(const RasterVertex& v0, const RasterVertex& v1, const RasterVertex& v2, Image* image)
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
            if (p.x < 0 || p.y < 0 || p.x >= m_context.m_width || p.y >= m_context.m_height)
            {
                continue;
            }

            Vec3f point = barycentric(p, v0.screenPos, v1.screenPos, v2.screenPos);
            if (point.x >= 0 && point.y >= 0 && point.z >= 0)
            {
                float z = v0.screenPos.z * point.x + v1.screenPos.z * point.y + v2.screenPos.z * point.z;
                uint32_t idx = p.x + p.y * m_context.m_width;
                if (z < m_context.m_depthBuffer[idx])
                {
                    m_context.m_depthBuffer[idx] = z;

                    if (image)
                    {
                        m_context.m_backBuffer[idx] = texel(v0, v1, v2, *image, point);
                    }
                    else
                    {
                        m_context.m_backBuffer[idx] = Utils::rasteriseColour(v0.colour, v1.colour, v2.colour, point.x, point.y, point.z);
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