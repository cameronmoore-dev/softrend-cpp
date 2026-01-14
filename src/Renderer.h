#pragma once

#include <initializer_list>
#include <cstring>
#include <vector>

#include "IShader.h"
#include "echomath.h"

namespace echo
{
    struct RenderTarget
    {
        rect rect;
        std::vector<uint32_t> colour;
        std::vector<float>    depth;
    };

    class Renderer
    {
    public:
        Renderer() = default;
        Renderer(void *windowRenderTarget);
    
        void bindVertexArray(const void* arr, const uint32_t size);
        void bindShader(IShader &shader);

        void clear(const uint32_t colour);
        void drawArray(const void* vertices, const uint32_t size);
        void drawIndex(const std::vector<uint32_t> &indices);
        void endDraw();
        
        Texture createTexture(const uint8_t *data, const int width, const int height, const int channels);
        void generateMipMaps(Texture &texture);
        void bilinearFilter(Texture &texture);
        void setVertexAttrib(const uint16_t size, const uint8_t index, const uint8_t count);
        void viewport(const int x, const int y, const int w, const int h);
        const rect &viewport() const;
        void setRenderTarget(const RenderTarget &target);
        void updateRenderTarget();
        RenderTarget &rendertarget();
        void internalDownScale(const uint32_t factor);
        uint32_t internalDownScale();

    private:
        RenderTarget          m_rendertarget;
        IShader              *m_shader = nullptr;
        std::vector<float>    m_vbo;
        std::vector<float>    m_copied;
        std::vector<float>    m_clipped;
        std::vector<float>    m_interpolated;
        std::vector<vertex>   m_input;
        std::vector<vertex>   m_output;
        std::vector<vec2i>    m_computedDownscale;

        rect  m_viewport = {};
        vec2f m_bbMin;
        vec2f m_bbMax;

        uint32_t m_scale = 1;

        /*  
            Pointers to shared window info,
            in order to have the up-to-date memory address for memcpys
            and screen size
        */
        void     *m_wndInfo = nullptr;
        uint32_t *m_drawbuffer = nullptr;
        uint32_t **m_ppBackbuffer = nullptr;
        uint16_t m_scrnwidth  = 0;
        uint16_t m_scrnheight = 0;
        
    private:
        void draw(Face &face);
        void transformVertex(vertex vertex);
        void rasterize(const Face &Face);
        void boundingBox(const vec4f& a, const vec4f& b, const vec4f& c);
        vec3f barycentric(const vec2i& p, const vec4f& a, const vec4f& b, const vec4f& c);
        float windingOrder(const vec4f& a, const vec4f& b, const vec4f& c);
        void clipTriangle(std::vector<vertex> &Face, std::vector<vertex> &out, int plane);
        vertex interpolate(const vertex a, const vertex b, const int plane);
        Face copyVertices(std::vector<float> &copy, const vertex v0, const vertex v1, const vertex v2);
        void resizeBuffers();
        void precomputeUpscaling();
        
        // uint32_t fog(uint32_t idx, uint32_t fogColour);
    };
}