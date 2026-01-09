#pragma once

#include <vector>
#include <cstring>

#include "IShader.h"
#include "math.h"

namespace echo
{
    struct RenderTarget
    {
        
    };

    class Renderer
    {
    public:
        Renderer() = default;
        Renderer(const void *frontbuffer, const int8_t downScale = 1);
    
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

    private:
        std::vector<float>      m_depthbuffer;
        std::vector<float>      m_vbo;
        std::vector<float>      m_clipped;
        std::vector<vertex>     m_input;
        std::vector<vertex>     m_output;

        rect    m_viewport = {};
        vec2f   m_bbMin;
        vec2f   m_bbMax;

        /*  
            Pointers to a shared backbuffer,
            in order to have the up-to-date memory address for memcpys
        */
        uint32_t *m_backbuffer = nullptr;
        uint32_t **m_ppBackbuffer = nullptr;
        uint16_t *m_wndWidth  = nullptr;
        uint16_t *m_wndHeight = nullptr;
        int8_t  m_scale = 1;
        
    private:
        void transformVertex(vertex vertex);
        void rasterise(IShader &shader, const face &face);
        void boundingBox(const vec4f& a, const vec4f& b, const vec4f& c);
        vec3f barycentric(const vec2i& p, const vec4f& a, const vec4f& b, const vec4f& c);
        float windingOrder(const vec4f& a, const vec4f& b, const vec4f& c);
        void clipTriangle(std::vector<vertex> &face, std::vector<vertex> &out, int plane);
        vertex interpolate(const vertex a, const vertex b, const int plane);
        
        // uint32_t fog(uint32_t idx, uint32_t fogColour);
    };
}