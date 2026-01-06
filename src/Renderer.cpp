#include "Renderer.h"

#include <algorithm>
#include <iterator>
#include <initializer_list>
#include <cmath>
#include <cstring>

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
    m_input.reserve(6);
    m_output.reserve(6);

    // vbo.reserve(17'000);

    // depthMap.resize(1024*1024, 1.f);
    previous = getTimeSeconds();

    /*  
        Navigating through the void, 
        assigning values based on the known layout of the EchoBufferInfo structure
    */
    m_viewportWidth = **((uint16_t**)bufferInfo);
    m_viewportHeight = **((uint16_t**)bufferInfo + 1);
    m_backbuffer = *((uint32_t**)bufferInfo + 2);
    m_depthbuffer = *((float**)bufferInfo + 3);

    m_ppBackBuffer = ((uint32_t**)bufferInfo + 2);
    m_ppDepthBuffer = ((float**)bufferInfo + 3);
}

void Renderer::clear(uint32_t colour)
{
    /*  
        If the array that stores the backbuffer has reallocated its memory,
        then set this pointer of the backbuffer to the new location.
    */
    if (m_backbuffer != *m_ppBackBuffer)
    {
        m_backbuffer = *m_ppBackBuffer;
        m_depthbuffer = *m_ppDepthBuffer;
    }

    std::fill(m_backbuffer, m_backbuffer + (m_viewportWidth * m_viewportHeight), colour);
    std::fill(m_depthbuffer, m_depthbuffer + (m_viewportWidth * m_viewportHeight), std::numeric_limits<float>::infinity());
}

void Renderer::viewport(int x, int y, int w, int h)
{
    m_viewportWidth  = w;
    m_viewportHeight = h;
}

void Renderer::draw(const void* vertices, int size, IShader& shader)
{
    m_vbo.resize(_priv::vertexdef.stride * size);
    std::memcpy(m_vbo.data(), vertices, _priv::vertexdef.size * size);

    m_clipped.clear();
    m_clipped.reserve(_priv::vertexdef.stride * NUM_PLANES);

    shader.reset();

    double current = getTimeSeconds();
    dt += current - previous;
    previous = current;

    // viewportWidth   = renderMode == RenderMode::Shadow ? 1024 : m_context.m_width;
    // viewportHeight  = renderMode == RenderMode::Shadow ? 1024 : m_context.m_height;
    
    for (uint32_t i = 0; i < size; i+=3)
    {
        int index = i * _priv::vertexdef.stride;
        int v0 = index;
        int v1 = index +  _priv::vertexdef.stride;
        int v2 = index + (_priv::vertexdef.stride * 2);

        shader.vertex(&m_vbo[v0]);
        shader.vertex(&m_vbo[v1]);
        shader.vertex(&m_vbo[v2]);
        vertex_t _v0 = &m_vbo[v0];
        vertex_t _v1 = &m_vbo[v1];
        vertex_t _v2 = &m_vbo[v2];
        
        m_input = { _v0, _v1, _v2 };
        for (size_t p = 0; p < NUM_PLANES; p++)
        {
            m_output.clear();

            clipTriangle(m_input, m_output, p);

            if (m_output.empty())
            {
                m_input.clear();
                break;
            }

            m_input.swap(m_output);
        }
        
        for (uint32_t j = 1; j + 1 < m_input.size(); j++)
        {
            /* Copy the vertices so transforming doesn’t corrupt shared data */
            float v0[_priv::vertexdef.stride] = {};
            float v1[_priv::vertexdef.stride] = {};
            float v2[_priv::vertexdef.stride] = {};
            std::memcpy(v0, m_input[0],   _priv::vertexdef.size);
            std::memcpy(v1, m_input[j],   _priv::vertexdef.size);
            std::memcpy(v2, m_input[j+1], _priv::vertexdef.size);

            transformVertex(v0);
            transformVertex(v1);
            transformVertex(v2);

            std::array<Vec4f, 3> positions = shader.getFaceAttrib4({ v0, v1, v2 }, 0);

            float winding = edgeFunction(positions[0], positions[1], positions[2]);
            if (winding <= 0.f)
            {
                continue;
            }

            face_t face = { v0, v1, v2 };
            rasterise(shader, face);
        }
    }

    // if (a)
    // {
    //     Image dm;
    //     dm.fromDepthMap(depthMap, 1024, 1024);
    //     dm.write("m_output.tga");
    //     a=false;
    // }
}

void Renderer::transformVertex(vertex_t vertex)
{
    float &vx = vertex[0];
    float &vy = vertex[1];
    float &vz = vertex[2];
    float &vw = vertex[3];

    /* Perspective Division */
    vx /= vw;
    vy /= vw;
    vz /= vw;

    /* To Screen Space Coordinates */
    Vec4f screenPos;
    screenPos.x = (vx + 1.f) * 0.5f * m_viewportWidth;
    screenPos.y = (vy + 1.f) * 0.5f * m_viewportHeight;
    screenPos.z = vz;

    /*  
        The extra '- 0.1f' is to stop flickering along the left and bottom screen edges after polygon clipping, 
        most likely due to floating-point precision issues.
        Might not be scalable with different screen resolutions. Test this later
    */
    vx = screenPos.x - 0.1f;
    vy = screenPos.y - 0.1f;
}

void Renderer::rasterise(IShader& shader, const face_t &face)
{
    std::array<Vec4f, 3> positions = shader.getFaceAttrib4(face, 0);

    boundingBox(positions[0], positions[1], positions[2]);

    Vec2i p;
    for (p.y = m_bbMin.y; p.y <= m_bbMax.y; p.y++)
    {
        for (p.x = m_bbMin.x; p.x <= m_bbMax.x; p.x++)
        {
            if (p.x < 0 || p.y < 0 || p.x >= m_viewportWidth || p.y >= m_viewportHeight)
            {
                continue;
            }

            Vec3f point = barycentric(p, positions[0], positions[1], positions[2]);
            if (point.x >= 0 && point.y >= 0 && point.z >= 0)
            {
                float z = positions[0].z * point.x + positions[1].z * point.y + positions[2].z * point.z;
                uint32_t idx = p.x + p.y * (m_viewportWidth);
                
                if (renderMode == RenderMode::Shadow)
                {
                    if (z < depthMap[idx])
                    {
                        depthMap[idx] = z;
                    }
                }
                else if (renderMode == RenderMode::Final)
                {
                    Vec4f fragColour;
                    if (z < m_depthbuffer[idx] && shader.fragment(point, face, fragColour))
                    {
                        m_depthbuffer[idx] = z;
                        // Vec4f src = { fragColour.x * fragColour.w, fragColour.y * fragColour.w, fragColour.z * fragColour.w, fragColour.w };
                        // Vec4f dest = Utils::hexToRGBA(m_backBuffer[idx]);
                        // Vec4f blendColour = src + dest * (1.f - src.w);

                        // m_backBuffer[idx] = Utils::rgbaToHex(blendColour);

                        uint8_t r = math::linearToSRGB(fragColour.x);
                        uint8_t g = math::linearToSRGB(fragColour.y);
                        uint8_t b = math::linearToSRGB(fragColour.z);
                        uint8_t a = math::linearToSRGB(fragColour.w);

                        m_backbuffer[idx] = (r<<16) | (g<<8) | b | (a<<24);

                        // int R = std::clamp(int(fragColour.x * 255.f), 0, 255);
                        // int G = std::clamp(int(fragColour.y * 255.f), 0, 255);
                        // int B = std::clamp(int(fragColour.z * 255.f), 0, 255);
                        // int A = std::clamp(int(fragColour.w * 255.f), 0, 255);

                        // m_backbuffer[idx] = (R<<16|G<<8|B|A<<24);
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

vertex_t Renderer::interpolate(const vertex_t a, const vertex_t b, const int plane)
{
    float t = 1.f;

    std::array<float, 4> aPos;
    std::array<float, 4> bPos;
    std::memcpy(aPos.data(), &a[0], sizeof(float) * aPos.size());
    std::memcpy(bPos.data(), &b[0], sizeof(float) * bPos.size());

    switch (plane)
    {
        case LEFT_PLANE:    t = (-aPos[0] - aPos[3]) / ((bPos[0] - aPos[0]) + (bPos[3] - aPos[3])); break;
        case RIGHT_PLANE:   t =  (aPos[3] - aPos[0]) / ((bPos[0] - aPos[0]) - (bPos[3] - aPos[3])); break;
        case BOTTOM_PLANE:  t = (-aPos[1] - aPos[3]) / ((bPos[1] - aPos[1]) + (bPos[3] - aPos[3])); break;
        case TOP_PLANE:     t =  (aPos[3] - aPos[1]) / ((bPos[1] - aPos[1]) - (bPos[3] - aPos[3])); break;
        case NEAR_PLANE:    t = (-aPos[2] - aPos[3]) / ((bPos[2] - aPos[2]) + (bPos[3] - aPos[3])); break;
        case FAR_PLANE:     t =  (aPos[3] - aPos[2]) / ((bPos[2] - aPos[2]) - (bPos[3] - aPos[3])); break;
        default:            break;
    }

    for (int i = 0; i < _priv::vertexdef.stride; i++)
    {
        m_clipped.emplace_back(a[i] + (b[i] - a[i]) * t);
    }

    int index = m_clipped.size() - _priv::vertexdef.stride;
    return &m_clipped[index];

    // glVertex out;
    // out.position = a.position + (b.position - a.position) * t;
    // out.normal = a.normal + (b.normal - a.normal) * t;
    // out.uv = a.uv + (b.uv - a.uv) * t;
    // out.tangent = a.tangent + (b.tangent - a.tangent) * t;
    // out.bitangent = a.bitangent + (b.bitangent - a.bitangent) * t;
    // out.colour = a.colour + (b.colour - a.colour) * t;

    // return out;
}

void Renderer::clipTriangle(std::vector<vertex_t> &face, std::vector<vertex_t> &out, const int plane)
{
    auto isInside = [](const vertex_t v, const int plane)
    {
        Vec4f pos = { v[0], v[1], v[2], v[3] };
        switch (plane)
        {
            case LEFT_PLANE:    return pos.x >= -pos.w;
            case RIGHT_PLANE:   return pos.x <= pos.w;
            case BOTTOM_PLANE:  return pos.y >= -pos.w;
            case TOP_PLANE:     return pos.y <= pos.w;
            case NEAR_PLANE:    return pos.z >= -pos.w;
            case FAR_PLANE:     return pos.z <= pos.w;
            default:            return false;
        }
    };

    for (size_t v = 0; v < face.size(); v++)
    {
        vertex_t curr = face[v];
        vertex_t next = face[(v + 1) % face.size()];

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

Texture Renderer::createTexture(uint8_t *data, int width, int height, int channels)
{
    MipMap mip =
    {
        .width = (uint16_t)width,
        .height = (uint16_t)height,
    };

    int size = (width * height) * channels;
    for (int i = 0; i < size; i += channels)
    {
        uint32_t pixel = (data[i] << 16 | data[i + 1] << 8 | data[i + 2]);
        if (channels == 4)
        {
            pixel |= data[i + 3] << 24;
        }
        
        mip.data.emplace_back(pixel);
    }

    Texture texture;
    texture.mipLevels.emplace_back(mip);

    return texture;
}

void Renderer::generateMipMaps(Texture &texture)
{
    uint8_t totalMipLevels = log2f(texture.mipLevels[0].width);
    texture.mipLevels.reserve(totalMipLevels + 1);

    auto blend = [](const std::initializer_list<uint32_t> colours)
    {
        uint32_t r = 0;
        uint32_t g = 0;
        uint32_t b = 0;
        uint32_t a = 0;

        for (const uint32_t &c : colours)
        {
            r += (c >> 16) & 0xff;
            g += (c >> 8)  & 0xff;
            b += (c >> 0)  & 0xff;
            a += (c >> 24) & 0xff;
        }

        uint8_t total = colours.size();
        uint32_t blended =  (r / total) << 16 | 
                            (g / total) << 8  | 
                            (b / total) << 0  | 
                            (a / total) << 24;
        return blended;
    };

    MipMap current = texture.mipLevels[0];
    while (current.width > 1 && current.height > 1)
    {
        uint16_t newWidth  = std::max(1, current.width  / 2);
        uint16_t newHeight = std::max(1, current.height / 2);

        MipMap next;
        next.width  = newWidth;
        next.height = newHeight;
        next.data.resize(newWidth * newHeight);

        for (int y = 0; y < newHeight; y++)
        {
            for (int x = 0; x < newWidth; x++)
            {
                uint32_t a = current.data[(x * 2) + (y * 2) * current.width];
                uint32_t b = current.data[(x * 2 + 1) + (y * 2) * current.width];
                uint32_t c = current.data[(x * 2) + (y * 2 + 1) * current.width];
                uint32_t d = current.data[(x * 2 + 1) + (y * 2 + 1) * current.width];

                next.data[x + y * newWidth] = blend({ a, b, c, d });
            }
        }

        texture.mipLevels.emplace_back(next);
        current = std::move(next);
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