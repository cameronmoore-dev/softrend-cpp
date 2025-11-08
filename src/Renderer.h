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

    Renderer(Window& context);
    void draw(const std::vector<Vertex>& vertices, IShader& shader);

    inline static std::vector<float> depthMap;
    
    RenderMode renderMode = RenderMode::Final;
private:
    Window& m_context;

    Vec2f m_bbMin;
    Vec2f m_bbMax;
    bool a = true;

    uint32_t viewportWidth;
    uint32_t viewportHeight;

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