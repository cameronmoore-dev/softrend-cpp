#pragma once

#include "jawl/base_window.h"
#include "../Scene.h"
#include "Renderer.h"
#include "Matrix.h"
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
    void onWindowResize(int w, int h);

private:
    jawl::Window &m_window;
    Renderer m_renderer;
    Model m_suzanne = Model("./resources/model_suzanne.obj");

    Texture m_texture;
    Texture m_normalMap;
    Texture m_specularMap;

    Camera m_camera;

    Suzanne_Shader shader;
    SSAO_Shader m_ssao;

    // Mat4 m_lightProj = Matrix::orthographic(-2.f, 2.f, -2.f, 2.f, 1.f, 10.f);
    // Mat4 m_lightView = Matrix::identity();

    Mat4 m_model = Matrix::identity();

    Vec3f position = {};
    float m_angle = 0;
    double m_dt = 0.f;

    bool a = true;
    bool ssao = false;

    bool fs = false;
};