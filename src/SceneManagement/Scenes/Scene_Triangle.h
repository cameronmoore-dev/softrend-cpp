#pragma once

#include "../Scene.h"
#include "../../IShader.h"
#include "../../Renderer.h"
#include "../../Camera.h"
#include "../../Matrix.h"

#include "../../Shaders/Suzanne_Shader.h"

class Scene_Triangle : public Scene
{
public:
    Scene_Triangle(Window& window);

    void start() override;
    
protected:
    void processInput() override;
    void update() override;
    void render() override;
    void shutdown() override;

private:
    Window& m_window;
    Renderer m_renderer = Renderer(m_window);
    Camera m_camera;

    // Suzanne_Shader m_shader;

    Mat4 m_model = Matrix::identity();

    Mat4 m_translate = Matrix::identity();
    Mat4 m_rotation = Matrix::identity();
    Mat4 m_scale = Matrix::identity();

    std::vector<Vertex> m_triangle;

    float m_angle = 0.f;

    double m_dt = 0.0;
};