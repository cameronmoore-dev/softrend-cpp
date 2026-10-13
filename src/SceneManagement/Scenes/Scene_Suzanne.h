#pragma once

#include <stb_image/stb_image.h>

#include "AppContext.h"
#include "../Scene.h"
#include "Model.h"
#include "Camera.h"

#include "Shaders/Suzanne_Shader.h"
#include "Shaders/SSAO_Shader.h"

#include "SceneManagement/SceneManager.h"

class Scene_Suzanne : public Scene
{
public:
    Scene_Suzanne(AppContext &app);

    void start() override;
    
protected:
    void processInput() override;
    void update() override;
    void render() override;
    void shutdown() override;

private:
    AppContext &m_app;

    Model m_suzanne = Model("./resources/model_suzanne.obj");

    Texture m_texture;
    Texture m_normalMap;
    Texture m_specularMap;

    Camera m_camera;

    Suzanne_Shader m_shader;
    SSAO_Shader m_ssao;

    mat4 m_model = identity();

    float m_angle = 0.f;
    double m_dt = 0;
    bool ssao = false;
};