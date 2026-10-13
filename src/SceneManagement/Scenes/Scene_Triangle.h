#pragma once

#include "AppContext.h"
#include "../Scene.h"
#include "Camera.h"
#include "vertex.h"

#include "Shaders/TriangleShader.h"

class Scene_Triangle : public Scene
{
public:
    Scene_Triangle(AppContext &app);

    void start() override;
    
protected:
    void processInput() override;
    void update() override;
    void render() override;
    void shutdown() override;

private:
    AppContext &m_app;
    Camera m_camera;

    TriangleShader m_shader;
    std::vector<Vertex> m_triangle;
};