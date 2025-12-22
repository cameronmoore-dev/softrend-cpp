#pragma once

#include "../Scene.h"
#include "../../IShader.h"
#include "../../Window.h"
#include "../../Renderer.h"
#include "../../Matrix.h"
#include "../../Model.h"
#include "../../Camera.h"

#include "../../Shaders/Cube_Shader.h"
#include "../../Shaders/FXAA_Shader.h"

class Scene_Cube : public Scene
{
public:
    Scene_Cube(Window &window, Renderer &renderer);

    void start() override;
    
protected:
    void processInput() override;
    void update() override;
    void render() override;
    void shutdown() override;

private:
    Window &m_window;
    Renderer &m_renderer;
    Model m_cube = Model("./resources/model_cube.obj");
    Image m_texture = Image("./resources/texture_dirtcube.tga");
    Camera m_camera;

    Cube_Shader m_shader;
    FXAA_Shader m_fxaa;
    
    Mat4 m_model = Matrix::identity();

    Mat4 m_translate = Matrix::identity();
    Mat4 m_rotation = Matrix::identity();
    Mat4 m_scale = Matrix::identity();

    float m_angle = 0.f;

    double m_dt = 0.f;
};