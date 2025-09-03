#pragma once

#include "../Scene.h"
#include "../../Window.h"
#include "../../Matrix.h"
#include "../../Model.h"

class Scene_Cube : public Scene
{
public:
    Scene_Cube(Window& window);

    void start() override;
    
protected:
    void processInput() override;
    void update() override;
    void render() override;
    void shutdown() override;

private:
    Window& m_window;
    Model m_cube = Model("./resources/model_cube.obj");
    Image m_texture = Image("./resources/texture_dirtcube.tga");

    Mat4 m_projection = Matrix::perspective(45.f, m_window.windowSize().x / m_window.windowSize().y, 0.1f, 100.f);
    //Mat4 m_projection = Matrix::orthographic(-3.2, 3.2, 2.4, -2.4, 0.1f, 100.f);
    Mat4 m_view = Matrix::translate({ 0, 0, -5.f });
    Mat4 m_model = Matrix::identity();

    float angle = 0.f;
};