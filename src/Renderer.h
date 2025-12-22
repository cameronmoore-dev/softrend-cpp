#pragma once

#include <vector>

#include "Window.h"
#include "IShader.h"
#include "Structs.h"
#include "Utils.h"

class Renderer
{
public:
    enum class RenderMode
    {
        Shadow,
        Final
    };

    Renderer(void *bufferInfo);
    void draw(const std::vector<Vertex>& vertices, IShader& shader);

    inline static std::vector<float> depthMap;
    
    RenderMode renderMode = RenderMode::Final;
private:
    /* Pointers to shared data between the Window and the Renderer through the EchoBufferInfo structure */
    uint32_t *m_backbuffer = nullptr;
    uint32_t **m_ppBackBuffer = nullptr;
    float *m_depthbuffer = nullptr;
    float **m_ppDepthBuffer = nullptr;

    uint16_t *m_width = nullptr;
    uint16_t *m_height = nullptr;

    Vec2f m_bbMin;
    Vec2f m_bbMax;
    bool a = true;

    uint16_t m_viewportWidth = 0;
    uint16_t m_viewportHeight = 0;
private:
    void transformVertex(glVertex& vertex);
    
    void rasterise(IShader& shader, const glTriangle& polygon);
    void boundingBox(const Vec4f& a, const Vec4f& b, const Vec4f& c);
    Vec3f barycentric(const Vec2i& p, const Vec4f& a, const Vec4f& b, const Vec4f& c);
    float edgeFunction(const Vec4f& a, const Vec4f& b, const Vec4f& c);

    void clipTriangle(std::vector<glVertex>& polygon, std::vector<glVertex>& out, int plane);
    glVertex interpolate(const glVertex& a, const glVertex& b, int plane);

    std::vector<glVertex> input;
    std::vector<glVertex> output;

    double dt = 0.0;
    double previous = 0.0;
    
    // uint32_t fog(uint32_t idx, uint32_t fogColour);
};