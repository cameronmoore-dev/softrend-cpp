#pragma once

#include "../Scene.h"
#include "../../Window.h"
#include "../../Matrix.h"
#include "../../Model.h"
#include "../../Camera.h"

class Scene_Suzanne : public Scene
{
public:
    Scene_Suzanne(Window& window);

    void start() override;
    
protected:
    void processInput() override;
    void update() override;
    void render() override;
    void shutdown() override;

private:
    Window& m_window;
    Model m_suzanne = Model("./resources/model_suzanne.obj");
    Image m_texture = Image("./resources/texture_suzanne.tga");
    Camera m_camera;

    Mat4 m_projection = Matrix::perspective(45.f, m_window.windowSize().x / m_window.windowSize().y, 0.5f, 100.f);
    //Mat4 m_projection = Matrix::orthographic(-3.2, 3.2, 2.4, -2.4, 0.1f, 100.f);
    Mat4 m_view = Matrix::identity();
    Mat4 m_model = Matrix::identity();

    Mat4 m_translate = Matrix::identity();
    Mat4 m_rotation = Matrix::identity();
    Mat4 m_scale = Matrix::identity();

    float m_angle = 0.f;

    double m_dt = 0.f;
};