#include "Renderer.h"

#include <algorithm>
#include <iterator>
#include <initializer_list>
#include <cmath>
#include <cstring>

#include <iostream>

namespace echo
{
    #define LEFT_PLANE      0
    #define RIGHT_PLANE     1
    #define BOTTOM_PLANE    2
    #define TOP_PLANE       3
    #define NEAR_PLANE      4
    #define FAR_PLANE       5
    #define NUM_PLANES      6

    Renderer::Renderer(const void *frontbuffer, const int8_t downScale) :
        m_scale(downScale)
    {
        m_wndWidth      = ((uint16_t*)frontbuffer);
        m_wndHeight     = ((uint16_t*)frontbuffer+1);
        m_backbuffer    = *((uint32_t**)frontbuffer+1);
        m_ppBackbuffer  = ((uint32_t**)frontbuffer+1);
    }
    
    void Renderer::clear(uint32_t colour)
    {
        /* Update where this pointer's memory address to the backbuffer if it changes */
        if (m_backbuffer != *m_ppBackbuffer)
        {
            m_backbuffer = *m_ppBackbuffer;
        }

        std::fill(m_backbuffer, m_backbuffer + (*m_wndWidth * *m_wndHeight), colour);
        std::fill(m_depthbuffer.data(), m_depthbuffer.data() + (*m_wndWidth * *m_wndHeight), std::numeric_limits<float>::max());
    }
    
    void Renderer::viewport(int x, int y, int w, int h)
    {
        m_viewport.x = x;
        m_viewport.y = y;
        m_viewport.width  = w;
        m_viewport.height = h;

        m_depthbuffer.resize(*m_wndWidth * *m_wndHeight);
    }

    void Renderer::setVertexAttrib(uint16_t size, uint8_t index, uint8_t count)
    {
        _priv::attrib_t attrib =
        {
            .index = index,
            .count = count
        };

        _priv::vertexdef.attribs.emplace_back(attrib);
        _priv::vertexdef.size = size;
        _priv::vertexdef.stride = size / sizeof(float);
    }
    
    void Renderer::draw(const void* vertices, int size, IShader& shader)
    {
        m_vbo.resize(_priv::vertexdef.stride * size);
        std::memcpy(m_vbo.data(), vertices, _priv::vertexdef.size * size);
    
        m_clipped.clear();
        shader.reset();

        for (uint32_t i = 0; i < size; i+=3)
        {
            int index = i * _priv::vertexdef.stride;
            int v0 = index;
            int v1 = index +  _priv::vertexdef.stride;
            int v2 = index + (_priv::vertexdef.stride * 2);
    
            shader.vertex(&m_vbo[v0]);
            shader.vertex(&m_vbo[v1]);
            shader.vertex(&m_vbo[v2]);
            vertex _v0 = &m_vbo[v0];
            vertex _v1 = &m_vbo[v1];
            vertex _v2 = &m_vbo[v2];
            
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
    
                std::array<vec4f, 3> positions = shader.getFaceAttrib<vec4f>({ v0, v1, v2 }, 0);
    
                float winding = windingOrder(positions[0], positions[1], positions[2]);
                if (winding <= 0.f)
                {
                    continue;
                }
    
                face face = { v0, v1, v2 };
                rasterise(shader, face);
            }
        }
    }
    
    void Renderer::transformVertex(vertex vertex)
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
        vx = (vx + 1.f) * (m_viewport.width  * 0.5f) + m_viewport.x;
        vy = (vy + 1.f) * (m_viewport.height * 0.5f) + m_viewport.y;
    }
    
    void Renderer::rasterise(IShader& shader, const face &face)
    {
        std::array<vec4f, 3> positions = shader.getFaceAttrib<vec4f>(face, 0);
        boundingBox(positions[0], positions[1], positions[2]);
    
        vec2i p;
        for (p.y = m_bbMin.y; p.y <= m_bbMax.y; p.y++)
        {
            for (p.x = m_bbMin.x; p.x <= m_bbMax.x; p.x++)
            {
                if (p.x <= m_viewport.x || p.x >= m_viewport.x + m_viewport.width  ||
                    p.y <= m_viewport.y || p.y >= m_viewport.y + m_viewport.height ||
                    p.x < 0 || p.x > *m_wndWidth  - 1 ||
                    p.y < 0 || p.y > *m_wndHeight - 1)
                {
                    continue;
                }
    
                vec3f point = barycentric(p, positions[0], positions[1], positions[2]);
                if (point.x >= 0 && point.y >= 0 && point.z >= 0)
                {
                    float z = positions[0].z * point.x + positions[1].z * point.y + positions[2].z * point.z;
                    uint32_t idx = p.x + p.y * *m_wndWidth;

                    vec4f fragColour;
                    if (z < m_depthbuffer[idx] && shader.fragment(point, face, fragColour))
                    {
                        m_depthbuffer[idx] = z;

                        echo::vec4f src = { fragColour.x * fragColour.w, fragColour.y * fragColour.w, fragColour.z * fragColour.w, fragColour.w };
                        echo::vec4f dest = hexToRGBA(m_backbuffer[idx]);
                        echo::vec4f blendColour = src + dest * (1.f - src.w);

                        m_backbuffer[idx] = rgbaToHex(blendColour);
                    }
                }
            }
        }
    }
    
    void Renderer::boundingBox(const vec4f& a, const vec4f& b, const vec4f& c)
    {
        float leftMost   = std::min(a.x, std::min(b.x, c.x));
        float bottomMost = std::min(a.y, std::min(b.y, c.y));
        float rightMost  = std::max(a.x, std::max(b.x, c.x));
        float topMost    = std::max(a.y, std::max(b.y, c.y));
    
        m_bbMin = { leftMost, bottomMost };
        m_bbMax = { rightMost, topMost };
    }
    
    vec3f Renderer::barycentric(const vec2i& p, const vec4f& a, const vec4f& b, const vec4f& c)
    {
        float denom = ((b.y-c.y)*(a.x-c.x)+(c.x-b.x)*(a.y-c.y));
        float w1 = ((b.y-c.y)*(p.x-c.x)+(c.x-b.x)*(p.y-c.y)) / denom;
        float w2 = ((c.y-a.y)*(p.x-c.x)+(a.x-c.x)*(p.y-c.y)) / denom;
        float w3 = 1.0f - w1 - w2;
    
        return { w1, w2, w3 };
    }
    
    float Renderer::windingOrder(const vec4f& a, const vec4f& b, const vec4f& c)
    {
        return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
    }
    
    vertex Renderer::interpolate(const vertex a, const vertex b, const int plane)
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
    }
    
    void Renderer::clipTriangle(std::vector<vertex> &face, std::vector<vertex> &out, const int plane)
    {
        auto isInside = [](const vertex v, const int plane)
        {
            vec4f pos = { v[0], v[1], v[2], v[3] };
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
            vertex curr = face[v];
            vertex next = face[(v + 1) % face.size()];
    
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
        texture.numChannels = channels;
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

    void Renderer::bilinearFilter(Texture &texture)
    {
        auto byte = [](const uint32_t colour, int shift)
        {
            return static_cast<uint8_t>((colour >> shift) & 0xFF);
        };
    
        auto lerp = [](const float fx, const float fy, const uint8_t c11, const uint8_t c12, const uint8_t c21, const uint8_t c22)
        {
            uint8_t a = (c11 * (1 - fx) + c21 * fx);
            uint8_t b = (c12 * (1 - fx) + c22 * fx);
            return (uint8_t)(a * (1 - fy) + b * fy);
        };
    
        for (int i = 0; i < texture.mipLevels.size(); i++)
        {
            MipMap &mip = texture.mipLevels[i];
            for (int y = 0; y < mip.height; y++)
            {
                for (int x = 0; x < mip.width; x++)
                {
                    float tx = x / float(mip.width - 1);
                    float ty = y / float(mip.height - 1);
                    int _x = (int)(tx * (mip.width - 1));
                    int _y = (int)(ty * (mip.height - 1));
                    float fx = tx * mip.width - _x;
                    float fy = ty * mip.height - _y;
    
                    /* -1 to the x and y creates an interesting dithering blur */
                    // int x1 = std::clamp(_x - 1, 0, mip.width - 1);
                    // int y1 = std::clamp(_y - 1, 0, mip.height - 1);
    
                    int x1 = std::clamp(_x, 0, mip.width - 1);
                    int y1 = std::clamp(_y, 0, mip.height - 1);
                    int x2 = std::clamp(_x + 1, 0, mip.width  - 1);
                    int y2 = std::clamp(_y + 1, 0, mip.height - 1);
                
                    const uint32_t &tl = mip.data[x1 + y1 * mip.width];
                    const uint32_t &tr = mip.data[x2 + y1 * mip.width];
                    const uint32_t &bl = mip.data[x1 + y2 * mip.width];
                    const uint32_t &br = mip.data[x2 + y2 * mip.width];
                
                    const uint8_t topleft[4]     = { byte(tl, 16), byte(tl, 8), byte(tl, 0), byte(tl, 24) };
                    const uint8_t topright[4]    = { byte(tr, 16), byte(tr, 8), byte(tr, 0), byte(tr, 24) };
                    const uint8_t bottomleft[4]  = { byte(bl, 16), byte(bl, 8), byte(bl, 0), byte(bl, 24) };
                    const uint8_t bottomright[4] = { byte(br, 16), byte(br, 8), byte(br, 0), byte(br, 24) };
                
                    uint8_t result[4] = {};
                    for (int i = 0; i < 4; i++)
                    {
                        result[i] = lerp(fx, fy, topleft[i], topright[i], bottomleft[i], bottomright[i]);
                    }
    
                    mip.data[x + y * mip.width] = ((result[0] << 16) | (result[1] << 8) | (result[2] << 0) | (result[3] << 24));
                }
            }
        }
    }

    uint32_t *Renderer::framebuffer()
    {
        return m_backbuffer;
    }

    float *Renderer::depthbuffer()
    {
        return m_depthbuffer.data();
    }

    void Renderer::size(uint16_t &outWidth, uint16_t &outHeight)
    {
        outWidth  = m_viewport.width;
        outHeight = m_viewport.height;
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
//             factor = 1.f / std::pow(e, distance * density);
//         }

//         vec3f rgb = Utils::hexToRGB(currentPixel);

//         /* Window clear colour */
//         vec3f fogColour = Utils::hexToRGB(0x111111);

//         rgb.x = rgb.x * factor + fogColour.x * (1.f - factor);
//         rgb.y = rgb.y * factor + fogColour.y * (1.f - factor);
//         rgb.z = rgb.z * factor + fogColour.z * (1.f - factor);

//         currentPixel = Utils::rgbToHex(rgb);
//     }
// }