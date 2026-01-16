#pragma once

#include <stb_image/stb_image.h>

#include "jawl/base_window.h"
#include "echo-renderer/renderer.h"
#include "../Scene.h"

#include "Camera.h"
#include "assimp_model.h"

#include "Shaders/AssimpModelShader.h"
#include "Shaders/DepthShader.h"

class Scene_Assimp : public Scene
{
public:
    Scene_Assimp(jawl::Window &window);

    void start() override;
    
protected:
    void processInput() override;
    void update() override;
    void render() override;
    void shutdown() override;

private:
    jawl::Window &m_window;
    echo::Renderer m_renderer;
    Camera m_camera;

    Camera m_lightSource;
    DepthShader m_dpthshdr;

    AssimpModelShader m_shader;
    AssimpModel m_model;
    AssimpModel m_grnd;
    
    echo::RenderTarget m_depthMap;
    echo::RenderTarget m_default;

    echo::Texture m_texture;

    echo::mat4 m_modelMat = echo::identity();

    double m_dt = 0.0;
    float angle = 0.f;
};
