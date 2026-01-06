#pragma once

#include <vector>

#include "IShader.h"
#include "Structs.h"
#include "math.h"

class Renderer
{
public:
    enum class RenderMode
    {
        Shadow,
        Final
    };

    Renderer() = default;
    Renderer(void *bufferInfo);

    void clear(uint32_t colour);
    void draw(const void* vertices, int size, IShader& shader);
    void viewport(int x, int y, int w, int h);

    void setVertexAttrib(uint16_t size, uint8_t index, uint8_t count)
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

    Texture createTexture(uint8_t *data, int width, int height, int channels);
    void generateMipMaps(Texture &texture);

    uint32_t *framebuffer() { return m_backbuffer; }
    float    *depthbuffer() { return m_depthbuffer; }
    void size(uint16_t &outWidth, uint16_t &outHeight)
    {
        outWidth  = m_viewportWidth;
        outHeight = m_viewportHeight;
    }

    inline static std::vector<float> depthMap;
    
    RenderMode renderMode = RenderMode::Final;

private:
    /* Pointers to shared data between the Window and the Renderer through the EchoBufferInfo structure */
    uint32_t *m_backbuffer = nullptr;
    uint32_t **m_ppBackBuffer = nullptr;
    float *m_depthbuffer = nullptr;
    float **m_ppDepthBuffer = nullptr;

    uint16_t m_viewportWidth = 0;
    uint16_t m_viewportHeight = 0;

    std::vector<float> m_vbo;

    Vec2f m_bbMin;
    Vec2f m_bbMax;
    bool a = true;

    bool b = false;
    
private:
    void transformVertex(vertex_t vertex);
    
    void rasterise(IShader &shader, const face_t &face);
    void boundingBox(const Vec4f& a, const Vec4f& b, const Vec4f& c);
    Vec3f barycentric(const Vec2i& p, const Vec4f& a, const Vec4f& b, const Vec4f& c);
    float edgeFunction(const Vec4f& a, const Vec4f& b, const Vec4f& c);

    void clipTriangle(std::vector<vertex_t> &face, std::vector<vertex_t> &out, int plane);
    vertex_t interpolate(const vertex_t a, const vertex_t b, const int plane);

    std::vector<vertex_t> m_input;
    std::vector<vertex_t> m_output;
    std::vector<float>    m_clipped;

    double dt = 0.0;
    double previous = 0.0;
    
    // uint32_t fog(uint32_t idx, uint32_t fogColour);
};