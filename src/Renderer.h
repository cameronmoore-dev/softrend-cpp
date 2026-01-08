#pragma once

#include <vector>
#include <cstring>

#include "IShader.h"
#include "math.h"

namespace echo
{
    class Renderer
    {
    public:
        Renderer() = default;
        Renderer(void *frontbuffer, const int8_t downScale = 1);
    
        void clear(uint32_t colour);
        void draw(const void* vertices, int size, IShader& shader);
        void viewport(int x, int y, int w, int h);
        void setVertexAttrib(uint16_t size, uint8_t index, uint8_t count);

        Texture createTexture(uint8_t *data, int width, int height, int channels);
        void generateMipMaps(Texture &texture);
        void bilinearFilter(Texture &texture);
    
        uint32_t *framebuffer();
        float *depthbuffer();
        void size(uint16_t &outWidth, uint16_t &outHeight);

        void upscale()
        {
            if (m_scale == 1)
            {
                std::memcpy(m_wndFrontbuffer, m_backbuffer.data(), m_viewportWidth * m_viewportHeight * sizeof(uint32_t));
                return;
            }
            
            uint32_t destWidth  = m_viewportWidth  * m_scale;
            uint32_t destHeight = m_viewportHeight * m_scale;
            for (int y = 0; y < destHeight; y++)
            {
                for (int x = 0; x < destWidth; x++)
                {
                    int srcx = std::floor(x * m_viewportWidth  / destWidth);
                    int srcy = std::floor(y * m_viewportHeight / destHeight);
                    m_wndFrontbuffer[x + y * destWidth] = m_backbuffer[srcx + srcy * m_viewportWidth];
                }
            }
        }

    private:
        /*  
            Pointers to the window's frontbuffer,
            in order to have the up-to-date memory address for memcpy's or image upscaling
        */
        uint32_t *m_wndFrontbuffer = nullptr;
        uint32_t **m_ppWndFrontbuffer = nullptr;

        std::vector<uint32_t> m_backbuffer;
        std::vector<float> m_depthbuffer;
        std::vector<float> m_vbo;

        int8_t   m_scale = 1;
        uint16_t m_viewportWidth = 0;
        uint16_t m_viewportHeight = 0;
    
        vec2f m_bbMin;
        vec2f m_bbMax;
        bool b = false;
        
    private:
        void transformVertex(vertex vertex);
        void rasterise(IShader &shader, const face &face);
        void boundingBox(const vec4f& a, const vec4f& b, const vec4f& c);
        vec3f barycentric(const vec2i& p, const vec4f& a, const vec4f& b, const vec4f& c);
        float windingOrder(const vec4f& a, const vec4f& b, const vec4f& c);
        void clipTriangle(std::vector<vertex> &face, std::vector<vertex> &out, int plane);
        vertex interpolate(const vertex a, const vertex b, const int plane);
    
        std::vector<vertex> m_input;
        std::vector<vertex> m_output;
        std::vector<float>  m_clipped;
    
        double dt = 0.0;
        double previous = 0.0;
        
        // uint32_t fog(uint32_t idx, uint32_t fogColour);
    };
}