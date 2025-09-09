#include "Renderer.h"

#include <algorithm>
#include <cmath>

#include <iostream>

Renderer::Renderer(Window& context) : 
    m_context(context)
{
    clipped.reserve(6);
}

void Renderer::draw(const std::vector<Vertex>& vertices, IShader& shader)
{
    for (uint32_t i = 0; i < vertices.size(); i+=3)
    {
        glTriangle polygon;
        shader.vertex(vertices[i],   polygon.v0);
        shader.vertex(vertices[i+1], polygon.v1);
        shader.vertex(vertices[i+2], polygon.v2);

        nearPlaneClip(polygon, clipped);
        for (uint32_t i = 0; i < clipped.size(); i+=3)
        {
            transformVertex(clipped[i]);
            transformVertex(clipped[i+1]);
            transformVertex(clipped[i+2]);

            glTriangle clippedPoly { clipped[i], clipped[i+1], clipped[i+2] };
            
            rasterise(shader, clippedPoly);
        }
    }
}

void Renderer::transformVertex(glVertex& vertex)
{
    Vec4f clip = vertex.position;

    /* For Perspective Corrected Texture Mapping */
    vertex.inverseW = 1.0f / clip.w;
    vertex.texCoords *= vertex.inverseW;

    /* Perspective Division */
    clip.x /= clip.w;
    clip.y /= clip.w;
    clip.z /= clip.w;

    /* To Screen Space Coordinates */
    Vec4f screenPos;
    screenPos.x = (clip.x + 1) * 0.5f * m_context.m_width;
    screenPos.y = (clip.y + 1) * 0.5f * m_context.m_height;
    screenPos.z = clip.z;

    vertex.position = screenPos;
}

void Renderer::rasterise(IShader& shader, const glTriangle& polygon)
{
    /* Back-face Culling */
    float winding = edgeFunction(polygon.v0.position, polygon.v1.position, polygon.v2.position);
    if (winding <= 0)
    {
        return;
    }

    boundingBox(polygon.v0.position, polygon.v1.position, polygon.v2.position);

    Vec2i p;
    for (p.y = m_bbMin.y; p.y <= m_bbMax.y; p.y++)
    {
        for (p.x = m_bbMin.x; p.x <= m_bbMax.x; p.x++)
        {
            if (p.x < 0 || p.y < 0 || p.x >= m_context.m_width || p.y >= m_context.m_height)
            {
                continue;
            }

            Vec3f point = barycentric(p, polygon.v0.position, polygon.v1.position, polygon.v2.position);
            if (point.x >= 0 && point.y >= 0 && point.z >= 0)
            {
                float z = polygon.v0.position.z * point.x + polygon.v1.position.z * point.y + polygon.v2.position.z * point.z;
                uint32_t idx = p.x + p.y * m_context.m_width;
                if (z < m_context.m_depthBuffer[idx])
                {
                    m_context.m_depthBuffer[idx] = z;
                    m_context.m_backBuffer[idx] = shader.fragment(point, polygon, z);
                }
            }
        }
    }
}

void Renderer::boundingBox(const Vec4f& a, const Vec4f& b, const Vec4f& c)
{
    float leftMost   = std::min(a.x, std::min(b.x, c.x));
    float bottomMost = std::min(a.y, std::min(b.y, c.y));
    float rightMost  = std::max(a.x, std::max(b.x, c.x));
    float topMost    = std::max(a.y, std::max(b.y, c.y));

    m_bbMin = { leftMost, bottomMost };
    m_bbMax = { rightMost, topMost };
}

Vec3f Renderer::barycentric(const Vec2i& p, const Vec4f& a, const Vec4f& b, const Vec4f& c)
{
    float denom = ((b.y-c.y)*(a.x-c.x)+(c.x-b.x)*(a.y-c.y));
    float w1 = ((b.y-c.y)*(p.x-c.x)+(c.x-b.x)*(p.y-c.y)) / denom;
    float w2 = ((c.y-a.y)*(p.x-c.x)+(a.x-c.x)*(p.y-c.y)) / denom;
    float w3 = 1.0f - w1 - w2;

    return { w1, w2, w3 };
}

float Renderer::edgeFunction(const Vec4f& a, const Vec4f& b, const Vec4f& c)
{
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

glVertex Renderer::interpolate(const glVertex& a, const glVertex& b)
{
    float t = (-a.position.z - a.position.w) / ((b.position.z - a.position.z) + (b.position.w - a.position.w));

    glVertex out;
    out.position = a.position + (b.position - a.position) * t;
    out.texCoords = a.texCoords + (b.texCoords - a.texCoords) * t;
    out.normal = a.normal + (b.normal - a.normal) * t;
    out.inverseW = a.inverseW + (b.inverseW - a.inverseW) * t;
    out.colour = a.colour + (b.colour - a.colour) * t;

    return out;
}

void Renderer::nearPlaneClip(glTriangle& polygon, std::vector<glVertex>& out)
{
    out.clear();

    glVertex& a = polygon.v0;
    glVertex& b = polygon.v1;
    glVertex& c = polygon.v2;

    glVertex triangle[3] = { a, b, c };

    auto isInside = [](const glVertex& v)
    {
        const Vec4f& p = v.position;
        return p.z >= -p.w;
    };

    bool inside[3] =
    {
        isInside(a),
        isInside(b),
        isInside(c)
    };

    int inCount = inside[0] + inside[1] + inside[2];

    if (inCount == 0)
    {
        return;
    }

    if (inCount == 3)
    {
        out.emplace_back(a);
        out.emplace_back(b);
        out.emplace_back(c);
        return;
    }

    if (inCount == 1)
    {
        int inVert = inside[0] ? 0 : inside[1] ? 1 : 2;

        glVertex& A = triangle[inVert];
        glVertex& B = triangle[(inVert + 1) % 3];
        glVertex& C = triangle[(inVert + 2) % 3];

        B = interpolate(A, B);
        C = interpolate(A, C);

        out.emplace_back(A);
        out.emplace_back(B);
        out.emplace_back(C);
        return;
    }

    if (inCount == 2)
    {
        int outVert = !inside[0] ? 0 : (!inside[1] ? 1 : 2);

        glVertex& C = triangle[outVert];
        glVertex& A = triangle[(outVert + 1) % 3];
        glVertex& B = triangle[(outVert + 2) % 3];

        glVertex newA = interpolate(A, C);
        glVertex newB = interpolate(B, C);

        out.emplace_back(A);
        out.emplace_back(B);
        out.emplace_back(newA);

        out.emplace_back(B);
        out.emplace_back(newB);
        out.emplace_back(newA);
        return;
    }
}


// uint32_t Renderer::fog(uint32_t idx, uint32_t fogColour)
// {
//     float depth = m_context.m_depthBuffer[idx];
//     if (depth <= 0.f || depth >= 1.f)
//     {
//         return fogColour;
//     }

//     return Utils::combineHex(m_context.m_backBuffer[idx], fogColour, 1);
// }

// void Renderer::linearFog(float zNear, float zFar, float fogStart, float fogEnd)
// {
//     _Fog(zNear, zFar, fogStart, fogEnd, 0);
// }

// void Renderer::exponentialFog(float zNear, float zFar, float density)
// {
//     _Fog(zNear, zFar, 0, 0, density);
// }

// void Renderer::_Fog(float zNear, float zFar, float fogStart, float fogEnd, float density)
// {
//     for (uint32_t i = 0; i < m_context.m_width*m_context.m_height; i++)
//     {
//         uint32_t& currentPixel = m_context.m_backBuffer[i];
//         float depth = m_context.m_depthBuffer[i];

//         /* 1.1f to give some leeway for objects clipping the far plane */
//         if (depth <= 0 || depth >= 1.1f)
//         {
//             continue;
//         }
        
//         float distance = (2.f * zNear * zFar) / (zFar + zNear - depth * (zFar - zNear));
//         float factor = 0.f;

//         if (density == 0)
//         {
//             /* Linear */
//             factor = (fogEnd - distance) / (fogEnd - fogStart);
//         }
//         else
//         {
//             /* Exponential */
//             factor = 1.f / std::pow(math::e, distance * density);
//         }

//         Vec3f rgb = Utils::hexToRGB(currentPixel);

//         /* Window clear colour */
//         Vec3f fogColour = Utils::hexToRGB(0x111111);

//         rgb.x = rgb.x * factor + fogColour.x * (1.f - factor);
//         rgb.y = rgb.y * factor + fogColour.y * (1.f - factor);
//         rgb.z = rgb.z * factor + fogColour.z * (1.f - factor);

//         currentPixel = Utils::rgbToHex(rgb);
//     }
// }