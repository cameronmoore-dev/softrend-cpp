#pragma once

#include "../Scene.h"
#include "IShader.h"
#include "Renderer.h"
#include "Camera.h"

#include "Shaders/Suzanne_Shader.h"

class Scene_Triangle : public Scene
{
public:
    Scene_Triangle(jawl::Window &window);

    void start() override;
    
protected:
    void processInput() override;
    void update() override;
    void render() override;
    void shutdown() override;

private:
    jawl::Window &m_window;
    echo::Renderer m_renderer;
    Camera m_camera;

    // Suzanne_Shader m_shader;

    echo::mat4 m_model = echo::identity();

    echo::mat4 m_translate = echo::identity();
    echo::mat4 m_rotation = echo::identity();
    echo::mat4 m_scale = echo::identity();

    std::vector<Vertex> m_triangle;

    float m_angle = 0.f;

    double m_dt = 0.0;
};