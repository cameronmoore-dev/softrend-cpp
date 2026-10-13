#pragma once

#include <stb_image/stb_image.h>

#include "AppContext.h"
#include "../Scene.h"

#include "Camera.h"
#include "assimp_model.h"

#include "Shaders/AssimpModelShader.h"
#include "Shaders/DepthShader.h"

#include "SceneManagement/SceneManager.h"

class Scene_Assimp : public Scene
{
public:
    Scene_Assimp(AppContext &app);

    void start() override;
    
protected:
    void processInput() override;
    void update() override;
    void render() override;
    void shutdown() override;

private:
    AppContext &m_app;
    Camera m_camera;

    Camera m_lightSource;
    DepthShader m_dpthshdr;

    AssimpModelShader m_shader;
    AssimpModel m_model;
    AssimpModel m_grnd;
    
    RenderTarget m_depthMap;
    RenderTarget m_default;

    Texture m_texture;

    mat4 m_modelMat = identity();

    double m_dt = 0.0;
    float angle = 0.f;
};
