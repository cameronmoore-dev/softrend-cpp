#pragma once

#include <vector>

#include "IShader.h"
#include "math.h"

namespace echo
{
    class Renderer
    {
    public:
        Renderer() = default;
        Renderer(void *bufferInfo);
    
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
        /* 
            Pointers to shared data between the Window and the Renderer through the EchoBufferInfo structure
        */
        uint32_t *m_backbuffer = nullptr;
        uint32_t **m_ppBackBuffer = nullptr;
        float *m_depthbuffer = nullptr;
        float **m_ppDepthBuffer = nullptr;
    
        uint16_t m_viewportWidth = 0;
        uint16_t m_viewportHeight = 0;
    
        std::vector<float> m_vbo;
    
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