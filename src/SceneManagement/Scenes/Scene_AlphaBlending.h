#pragma once

#include "../Scene.h"
#include "../../Window.h"
#include "../../Renderer.h"
#include "../../Model.h"
#include "../../Camera.h"

#include "../../Shaders/Grass_Shader.h"
#include "../../Shaders/Cube_Shader.h"
#include "../../Shaders/Window_Shader.h"

class Scene_AlphaBlending : public Scene
{
public:
    Scene_AlphaBlending(Window& window);

    void start() override;
    
protected:
    void processInput() override;
    void update() override;
    void render() override;
    void shutdown() override;

private:
    Window& m_window;
    Renderer m_renderer = Renderer(m_window);
    Camera m_camera;

    Grass_Shader m_grassShader;
    Cube_Shader m_cubeShader;
    Window_Shader m_windowShader;

    Model m_cube = Model("./resources/model_cube.obj");
    Model m_plane = Model("./resources/model_plane.obj");
    
    Image m_grassTexture = Image("./resources/texture_grass.tga");
    Image m_dirtTexture = Image("./resources/texture_dirtcube.tga");
    Image m_windowTexture = Image("./resources/texture_window.tga");

    Mat4 m_grassModel = Matrix::identity();
    Mat4 m_cubeModel = Matrix::identity();
    Mat4 m_windowModel = Matrix::identity();

    Mat4 m_grassTranslate = Matrix::identity();
    Mat4 m_cubeTranslate = Matrix::identity();
    Mat4 m_windowTranslate = Matrix::identity();

    Mat4 m_cubeRotation = Matrix::identity();

    Mat4 m_rotation = Matrix::identity();
    Mat4 m_scale = Matrix::identity();

    float m_angle = 0.f;
    double m_dt = 0.f;
};