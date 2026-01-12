#pragma once

#include <stb_image/stb_image.h>

#include "jawl/base_window.h"
#include "Renderer.h"
#include "../Scene.h"

#include "Camera.h"
#include "assimp_model.h"

#include "Shaders/AssimpModelShader.h"

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

    AssimpModelShader m_shader;
    AssimpModel m_model;
    
    echo::Texture m_texture;

    echo::mat4 m_modelMat = echo::identity();

    double m_dt = 0.0;
};
