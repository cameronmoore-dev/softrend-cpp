#pragma once

#include <stb_image/stb_image.h>

#include "AppContext.h"
#include "../Scene.h"
#include "Model.h"
#include "Camera.h"

#include "Shaders/Grass_Shader.h"
#include "Shaders/Cube_Shader.h"
#include "Shaders/Window_Shader.h"

#include "SceneManagement/SceneManager.h"

class Scene_AlphaBlending : public Scene
{
public:
    Scene_AlphaBlending(AppContext &app);

    void start() override;
    
protected:
    void processInput() override;
    void update() override;
    void render() override;
    void shutdown() override;

private:
    AppContext &m_app;
    Camera m_camera;

    RenderTarget m_rt;

    Grass_Shader    m_grassShader;
    Cube_Shader     m_cubeShader;
    Window_Shader   m_windowShader;

    Model m_cube  = Model("./resources/model_cube.obj");
    Model m_plane = Model("./resources/model_plane.obj");
    
    Texture m_grassTexture;
    Texture m_dirtTexture;
    Texture m_windowTexture;

    mat4 m_grassModel  = identity();
    mat4 m_cubeModel   = identity();
    mat4 m_windowModel = identity();

    float m_angle = 0.f;
    double m_dt = 0.f;
};