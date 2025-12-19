#pragma once

#include <vector>

#include "../Scene.h"
#include "Window.h"
#include "GLShader.h"

struct POS
{
    float x;
    float y;
};

struct COLOUR
{
    float r;
    float g;
    float b;
};

struct VERTEX
{
    POS pos;
    COLOUR colour;
};

class Scene_OpenGL : public Scene
{
public:
    Scene_OpenGL(Window &window);

    void start() override;
    
protected:
    void processInput() override;
    void update() override;
    void render() override;
    void shutdown() override;

private:
    Window &m_window;
    GLShader m_shader;

    std::vector<VERTEX> m_vertices;
    uint32_t m_vao = 0;
    uint32_t m_vbo = 0;

    double m_dt = 0.0;
};