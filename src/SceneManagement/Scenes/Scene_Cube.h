#pragma once

#include <stb_image/stb_image.h>

#include "AppContext.h"
#include "../Scene.h"
#include "Model.h"
#include "Camera.h"

#include "Shaders/Cube_Shader.h"
#include "Shaders/FXAA_Shader.h"

#include "SceneManagement/SceneManager.h"

class Scene_Cube : public Scene
{
public:
    Scene_Cube(AppContext &app);
    ~Scene_Cube() = default;

    void start() override;
    
protected:
    void processInput() override;
    void update() override;
    void render() override;
    void shutdown() override;

private:
    AppContext &m_app;
    Model m_cube = Model("./resources/model_cube.obj");
    Camera m_camera;

    Texture m_texture;

    Cube_Shader m_shader;
    FXAA_Shader m_fxaa;    
    
    mat4 m_model = identity();

    float m_angle = 0.f;
    double m_dt = 0.f;
    bool doFXAA = false;
};