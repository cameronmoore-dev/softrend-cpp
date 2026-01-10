#pragma once

#include <vector>
#include <cstring>

#include "IShader.h"
#include "math.h"

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
    
        void clear(uint32_t colour);
        void draw(const void* vertices, int size, IShader& shader);
        void viewport(int x, int y, int w, int h);
        void setVertexAttrib(uint16_t size, uint8_t index, uint8_t count);
        void setRenderTarget(RenderTarget &target)
        {
            m_currentTarget = target;
            updateRenderTarget();
        }
        void updateRenderTarget()
        {
            viewport(m_currentTarget.rect.x, 
                     m_currentTarget.rect.y, 
                     m_currentTarget.rect.width, 
                     m_currentTarget.rect.height);
        }

        void internalDownScale(uint8_t factor)
        {
            m_scale = factor;
        }
        const uint8_t internalDownScale() const
        {
            return m_scale;
        }

        RenderTarget &getCurrentRenderTarget()
        {
            return m_currentTarget;
        }

        void endDraw()
        {
            bool buffersEqualSize = (m_scrnwidth * m_scrnheight) == m_currentTarget.rect.width * m_currentTarget.rect.height;
            if (buffersEqualSize)
            {
                std::memcpy(m_drawbuffer, 
                            m_currentTarget.colour.data(), 
                            m_currentTarget.rect.width * m_currentTarget.rect.height * sizeof(uint32_t));
                return;
            }

            for (int y = 0; y < m_scrnheight; y++)
            {
                for (int x = 0; x < m_scrnwidth; x++)
                {
                    int sx = std::floor(x * m_currentTarget.rect.width  / m_scrnwidth);
                    int sy = std::floor(y * m_currentTarget.rect.height / m_scrnheight);

                    m_drawbuffer[x + y * m_scrnwidth] = m_currentTarget.colour[sx + sy * m_currentTarget.rect.width];
                }
            }
        }

        Texture createTexture(const uint8_t *data, const int width, const int height, const int channels);
        void generateMipMaps(Texture &texture);
        void bilinearFilter(Texture &texture);

        const rect &getViewport() const
        {
            return m_viewport;
        }

    private:
        RenderTarget        m_currentTarget;
        std::vector<float>  m_vbo;
        std::vector<float>  m_clipped;
        std::vector<vertex> m_input;
        std::vector<vertex> m_output;

        rect  m_viewport = {};
        vec2f m_bbMin;
        vec2f m_bbMax;

        uint8_t m_scale = 1;

        /*  
            Pointers to shared window info,
            in order to have the up-to-date memory address for memcpys
            and screen size
        */
        void *m_wndInfo = nullptr;
        uint32_t *m_drawbuffer = nullptr;
        uint32_t **m_ppBackbuffer = nullptr;
        uint16_t m_scrnwidth  = 0;
        uint16_t m_scrnheight = 0;
        
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