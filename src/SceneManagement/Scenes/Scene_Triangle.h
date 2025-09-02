#pragma once

#include "../Scene.h"
#include "../../Window.h"
#include "../../Matrix.h"

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

    Mat4 m_projection = Matrix::perspective(45.f, m_window.windowSize().x / m_window.windowSize().y, 0.1f, 100.f);
    //Mat4 m_projection = Matrix::orthographic(-3.2, 3.2, 2.4, -2.4, 0.1f, 100.f);
    Mat4 m_view = Matrix::translate({ 0, 0, -5.f });
    Mat4 m_model = Matrix::identity();

    Mat4 m_translate = Matrix::identity();
    Mat4 m_rotation = Matrix::identity();
    Mat4 m_scale = Matrix::identity();

    std::vector<Vertex> m_triangle;

    float m_angle = 0.f;
};