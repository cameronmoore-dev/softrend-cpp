#pragma once

#include "jawl/base_window.h"
#include "../Scene.h"
#include "Renderer.h"
#include "Model.h"
#include "Camera.h"

#include "Shaders/Grass_Shader.h"
#include "Shaders/Cube_Shader.h"
#include "Shaders/Window_Shader.h"

class Scene_AlphaBlending : public Scene
{
public:
    Scene_AlphaBlending(jawl::Window &window);

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

    Grass_Shader m_grassShader;
    Cube_Shader m_cubeShader;
    Window_Shader m_windowShader;

    Model m_cube = Model("./resources/model_cube.obj");
    Model m_plane = Model("./resources/model_plane.obj");
    
    // Image m_grassTexture = Image("./resources/texture_grass.tga");
    // Image m_dirtTexture = Image("./resources/texture_dirtcube.tga");
    // Image m_windowTexture = Image("./resources/texture_window.tga");

    echo::mat4 m_grassModel = echo::identity();
    echo::mat4 m_cubeModel = echo::identity();
    echo::mat4 m_windowModel = echo::identity();

    echo::mat4 m_grassTranslate = echo::identity();
    echo::mat4 m_cubeTranslate = echo::identity();
    echo::mat4 m_windowTranslate = echo::identity();

    echo::mat4 m_cubeRotation = echo::identity();

    echo::mat4 m_rotation = echo::identity();
    echo::mat4 m_scale = echo::identity();

    float m_angle = 0.f;
    double m_dt = 0.f;
};