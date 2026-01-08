#pragma once

#include "jawl/base_window.h"

#include "../Scene.h"
#include "IShader.h"
#include "Renderer.h"
#include "Model.h"
#include "Camera.h"

#include "Shaders/Cube_Shader.h"
#include "Shaders/FXAA_Shader.h"

class Scene_Cube : public Scene
{
public:
    Scene_Cube(jawl::Window &window);

    void start() override;
    
protected:
    void processInput() override;
    void update() override;
    void render() override;
    void shutdown() override;

private:
    jawl::Window &m_window;
    echo::Renderer m_renderer;
    Model m_cube = Model("./resources/model_cube.obj");
    Camera m_camera;

    echo::Texture m_texture;

    Cube_Shader m_shader;
    FXAA_Shader m_fxaa;    
    
    echo::mat4 m_model = echo::identity();

    float m_angle = 0.f;
    double m_dt = 0.f;
};