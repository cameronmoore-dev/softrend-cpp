#pragma once

#include <stb_image/stb_image.h>

#include "jawl/base_window.h"
#include "../Scene.h"
#include "Renderer.h"
#include "Model.h"
#include "Camera.h"

#include "Shaders/Suzanne_Shader.h"
#include "Shaders/SSAO_Shader.h"

class Scene_Suzanne : public Scene
{
public:
    Scene_Suzanne(jawl::Window &window);

    void start() override;
    
protected:
    void processInput() override;
    void update() override;
    void render() override;
    void shutdown() override;

private:
    jawl::Window &m_window;
    echo::Renderer m_renderer;
    Model m_suzanne = Model("./resources/model_suzanne.obj");

    echo::Texture m_texture;
    echo::Texture m_normalMap;
    echo::Texture m_specularMap;

    Camera m_camera;

    Suzanne_Shader m_shader;
    SSAO_Shader m_ssao;

    echo::mat4 m_model = echo::identity();

    float m_angle = 0.f;
    double m_dt = 0;

    bool a = true;
    bool ssao = false;

    bool fs = false;
};