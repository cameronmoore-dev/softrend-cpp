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
    
    RenderMode renderMode = RenderMode::Shadow;
private:
    Window& m_context;

    Vec2f m_bbMin;
    Vec2f m_bbMax;
    bool a = true;

    uint32_t viewportWidth;
    uint32_t viewportHeight;

    std::vector<glVertex> clipped;

private:
    void transformVertex(glVertex& vertex);
    
    void rasterise(IShader& shader, const glTriangle& polygon);
    void boundingBox(const Vec4f& a, const Vec4f& b, const Vec4f& c);
    Vec3f barycentric(const Vec2i& p, const Vec4f& a, const Vec4f& b, const Vec4f& c);
    float edgeFunction(const Vec4f& a, const Vec4f& b, const Vec4f& c);

    void nearPlaneClip(glTriangle& polygon, std::vector<glVertex>& out);
    glVertex interpolate(const glVertex& a, const glVertex& b);
    
    // uint32_t fog(uint32_t idx, uint32_t fogColour);
};