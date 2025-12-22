#include "Renderer.h"

#include <algorithm>
#include <cmath>

#include <iostream>

#define LEFT_PLANE      0
#define RIGHT_PLANE     1
#define BOTTOM_PLANE    2
#define TOP_PLANE       3
#define NEAR_PLANE      4
#define FAR_PLANE       5
#define NUM_PLANES      6

Renderer::Renderer(void *bufferInfo)
{
    input.reserve(6);
    output.reserve(6);

    // depthMap.resize(1024*1024, 1.f);
    previous = getTimeSeconds();

    /*  Navigating through the void, 
        assigning values based on the known layout of the EchoBufferInfo structure
    */
    m_width = *((uint16_t**)bufferInfo);
    m_height = *((uint16_t**)bufferInfo + 1);
    m_backbuffer = *((uint32_t**)bufferInfo + 2);
    m_depthbuffer = *((float**)bufferInfo + 3);

    m_ppBackBuffer = ((uint32_t**)bufferInfo + 2);
    m_ppDepthBuffer = ((float**)bufferInfo + 3);

    m_viewportWidth = *m_width;
    m_viewportHeight = *m_height;
}

void Renderer::draw(const std::vector<Vertex>& vertices, IShader& shader)
{
    /*  If the array that stores the backbuffer has reallocated its memory,
        then set this pointer of the backbuffer to the new location.
    */
    if (m_backbuffer != *m_ppBackBuffer)
    {
        m_backbuffer = *m_ppBackBuffer;
        m_depthbuffer = *m_ppDepthBuffer;
    }

    double current = getTimeSeconds();
    dt += current - previous;
    previous = current;

    // viewportWidth   = renderMode == RenderMode::Shadow ? 1024 : m_context.m_width;
    // viewportHeight  = renderMode == RenderMode::Shadow ? 1024 : m_context.m_height;
    
    for (uint32_t i = 0; i < vertices.size(); i+=3)
    {
        glTriangle polygon;
        shader.vertex(vertices[i],   polygon.v0);
        shader.vertex(vertices[i+1], polygon.v1);
        shader.vertex(vertices[i+2], polygon.v2);

        input = { polygon.v0, polygon.v1, polygon.v2 };
        for (size_t p = 0; p < NUM_PLANES; p++)
        {
            output.clear();

            clipTriangle(input, output, p);

            if (output.empty())
            {
                input.clear();
                break;
            }

            input.swap(output);
        }

        for (uint32_t j = 1; j + 1 < input.size(); j++)
        {
            /* Copy the vertices so transforming doesn’t corrupt shared data */
            glVertex v0 = input[0];
            glVertex v1 = input[j];
            glVertex v2 = input[j + 1];

            transformVertex(v0);
            transformVertex(v1);
            transformVertex(v2);

            float winding = edgeFunction(v0.position, v1.position, v2.position);
            if (winding <= 0.f)
            {
                continue;
            }

            glTriangle clippedPoly { v0, v1, v2 };
            rasterise(shader, clippedPoly);
        }
    }

    // if (a)
    // {
    //     Image dm;
    //     dm.fromDepthMap(depthMap, 1024, 1024);
    //     dm.write("output.tga");
    //     a=false;
    // }
}

void Renderer::transformVertex(glVertex& vertex)
{
    Vec4f clip = vertex.position;

    vertex.orig_uv = vertex.uv;

    /* For Perspective Corrected Texture Mapping */
    vertex.inverseW = 1.0f / clip.w;
    vertex.uv *= vertex.inverseW;

    /* Perspective Division */
    clip.x /= clip.w;
    clip.y /= clip.w;
    clip.z /= clip.w;

    /* To Screen Space Coordinates */
    Vec4f screenPos;
    screenPos.x = (clip.x + 1.f) * 0.5f * m_viewportWidth;
    screenPos.y = (clip.y + 1.f) * 0.5f * m_viewportHeight;
    screenPos.z = clip.z;

    /* The extra - 0.1f is to stop flickering along the left and bottom screen edges after polygon clipping, most likely due to floating-point precision issues */
    /* Might not be scalable with different screen resolutions. Test this later */
    vertex.position = screenPos - 0.1f;
}

void Renderer::rasterise(IShader& shader, const glTriangle& polygon)
{
    boundingBox(polygon.v0.position, polygon.v1.position, polygon.v2.position);

    Vec2i p;
    for (p.y = m_bbMin.y; p.y <= m_bbMax.y; p.y++)
    {
        for (p.x = m_bbMin.x; p.x <= m_bbMax.x; p.x++)
        {
            if (p.x < 0 || p.y < 0 || p.x >= m_viewportWidth || p.y >= m_viewportHeight)
            {
                continue;
            }

            Vec3f point = barycentric(p, polygon.v0.position, polygon.v1.position, polygon.v2.position);
            if (point.x >= 0 && point.y >= 0 && point.z >= 0)
            {
                float z = polygon.v0.position.z * point.x + polygon.v1.position.z * point.y + polygon.v2.position.z * point.z;
                uint32_t idx = p.x + p.y * (*m_width);
                
                if (renderMode == RenderMode::Shadow)
                {
                    if (z < depthMap[idx])
                    {
                        depthMap[idx] = z;
                    }
                }
                else if (renderMode == RenderMode::Final)
                {
                    Vec4f linearColour;
                    if (z < m_depthbuffer[idx] && shader.fragment(point, polygon, linearColour))
                    {
                        m_depthbuffer[idx] = z;
                        // Vec4f src = { linearColour.x * linearColour.w, linearColour.y * linearColour.w, linearColour.z * linearColour.w, linearColour.w };
                        // Vec4f dest = Utils::hexToRGBA(m_context.m_backBuffer[idx]);
                        // Vec4f blendColour = src + dest * (1.f - src.w);

                        // m_context.m_backBuffer[idx] = Utils::rgbaToHex(blendColour);

                        uint8_t r = Utils::linearToSRGB(linearColour.x);
                        uint8_t g = Utils::linearToSRGB(linearColour.y);
                        uint8_t b = Utils::linearToSRGB(linearColour.z);

                        m_backbuffer[idx] = (r<<16) | (g<<8) | b;

                        // m_context.m_backBuffer[idx] = Utils::rgbaToHex(linearColour);
                    }
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

glVertex Renderer::interpolate(const glVertex& a, const glVertex& b, const int plane)
{
    float t = 1.f;

    switch (plane)
    {
        case LEFT_PLANE:    t = (-a.position.x - a.position.w) / ((b.position.x - a.position.x) + (b.position.w - a.position.w)); break;
        case RIGHT_PLANE:   t =  (a.position.w - a.position.x) / ((b.position.x - a.position.x) - (b.position.w - a.position.w)); break;
        case BOTTOM_PLANE:  t = (-a.position.y - a.position.w) / ((b.position.y - a.position.y) + (b.position.w - a.position.w)); break;
        case TOP_PLANE:     t =  (a.position.w - a.position.y) / ((b.position.y - a.position.y) - (b.position.w - a.position.w)); break;
        case NEAR_PLANE:    t = (-a.position.z - a.position.w) / ((b.position.z - a.position.z) + (b.position.w - a.position.w)); break;
        case FAR_PLANE:     t =  (a.position.w - a.position.z) / ((b.position.z - a.position.z) - (b.position.w - a.position.w)); break;
        default:            break;
    }

    glVertex out;
    out.position = a.position + (b.position - a.position) * t;
    out.uv = a.uv + (b.uv - a.uv) * t;
    out.normal = a.normal + (b.normal - a.normal) * t;
    out.tangent = a.tangent + (b.tangent - a.tangent) * t;
    out.bitangent = a.bitangent + (b.bitangent - a.bitangent) * t;
    out.colour = a.colour + (b.colour - a.colour) * t;

    return out;
}

void Renderer::clipTriangle(std::vector<glVertex>& polygon, std::vector<glVertex>& out, const int plane)
{
    auto isInside = [](const glVertex& v, const int plane)
    {
        const Vec4f& p = v.position;
        switch (plane)
        {
            case LEFT_PLANE:    return p.x >= -p.w;
            case RIGHT_PLANE:   return p.x <= p.w;
            case BOTTOM_PLANE:  return p.y >= -p.w;
            case TOP_PLANE:     return p.y <= p.w;
            case NEAR_PLANE:    return p.z >= -p.w;
            case FAR_PLANE:     return p.z <= p.w;
            default:            return false;
        }
    };

    for (size_t v = 0; v < polygon.size(); v++)
    {
        const glVertex& curr = polygon[v];
        const glVertex& next = polygon[(v + 1) % polygon.size()];

        bool currInside = isInside(curr, plane);
        bool nextInside = isInside(next, plane);

        if (currInside && nextInside)
        {
            out.emplace_back(next);
        }

        else if (currInside && !nextInside)
        {
            out.emplace_back(interpolate(curr, next, plane));
        }

        else if (!currInside && nextInside)
        {
            out.emplace_back(interpolate(curr, next, plane));
            out.emplace_back(next);
        }
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