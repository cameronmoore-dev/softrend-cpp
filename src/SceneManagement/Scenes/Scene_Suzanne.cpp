#include "Scene_Suzanne.h"

Scene_Suzanne::Scene_Suzanne(jawl::Window &window) :
    m_window(window)
{
    int x = 0;
    int y = 0;
    m_window.getSize(x, y);
    m_camera = Camera(45.f, (float)x / (float)y, 0.5f, 20.f);

    m_renderer = Renderer(m_window.getEBI());
}

void Scene_Suzanne::start()
{
    shader.setUniform<Mat4>(shader.model, m_model);
    shader.setUniform<Mat4>(shader.view, m_camera.view());
    shader.setUniform<Mat4>(shader.projection, m_camera.projection());

    shader.setUniform<Image>(shader.diffuseMap, m_texture);
    shader.setUniform<Image>(shader.normalMap, m_normalMap);
    shader.setUniform<Image>(shader.specularMap, m_specularMap);

    // m_ssao.fb = &m_window.m_backBuffer;
    // m_ssao.zbuffer = &m_window.m_depthBuffer;

    // int ox;
    // int oy;
    // m_window.getSize(ox, oy);
    
    // m_ssao.width = ox;
    // m_ssao.height = oy;
    // m_ssao.initAOBuffer();

    // Vec3f pos{ 0.f, 0.f, -5.f };
    // Vec3f forward{ 0.f, 0.f, -1.f };
    // m_lightView = Matrix::lookAt(pos, pos+forward, { 0, 1, 0 });

    // shader.lightProj = m_lightProj;
    // shader.lightView = m_lightView;

    double previous = getTimeSeconds();
    while (m_window.isOpen())
    {
        double current = getTimeSeconds();
        m_dt = current-previous;
        previous = current;

        processInput();
        update();
        render();

        printf("FPS: %.0f\n", 1.f/m_dt);
    }

    m_window.cleanup();
}

void Scene_Suzanne::processInput()
{
    jawl::Event e;
    while (m_window.pollEvents(e))
    {
        switch(e.type)
        {
            case jawl::Event::WindowResize:
            {
                onWindowResize(e.wndResizeInfo.width, e.wndResizeInfo.height);
            } break;

            case jawl::Event::KeyPressed:
            {
                if (e.keyCode == jawl::getKey(jawl::Key::Escape))
                {
                    m_window.close();
                }

                else if (e.keyCode == jawl::getKey(jawl::Key::F5))
                {
                    fs = !fs;
                    m_window.setFullscreen(fs);
                }

                else if (e.keyCode == jawl::getKey(jawl::Key::Backspace))
                {
                    ssao = !ssao;
                }

                else if (e.keyCode == jawl::getKey(jawl::Key::Enter))
                {
                    m_ssao.doBlur = !m_ssao.doBlur;
                }
            } break;

            case jawl::Event::KeyReleased:
            {
            } break;

            case jawl::Event::MouseMove:
            {
                m_camera.updateRotation(e.mousePos.x, e.mousePos.y, m_dt);
            } break;

            case jawl::Event::GamepadButtonPressed:
            {
                if (e.gamepadButton ==jawl::getKey(jawl::Key::A))
                {
                    printf("A pressed\n");
                }
            } break;

            case jawl::Event::GamepadButtonReleased:
            {
                if (e.gamepadButton == jawl::getKey(jawl::Key::A))
                {
                    printf("A released\n");
                }
            } break;

            case jawl::Event::GamepadAxisMoved:
            {
            } break;

            case jawl::Event::GamepadTrigger:
            {
                if (e.triggerInfo.value < jawl::GamepadDeadzone::Trigger)
                {
                    break;
                }

                if (e.triggerInfo.trigger == 0)
                {
                    printf("lt: %d\n", e.axisInfo.value);
                }

                if (e.triggerInfo.trigger == 1)
                {
                    printf("rt: %d\n", e.axisInfo.value);
                }
            } break;
        }
    }
}

void Scene_Suzanne::update()
{
    m_camera.updateMovement(m_dt);
    if (jawl::isGamepadConnected(0))
    {
        m_camera.updateRotation(
             jawl::controllerAxisValue(0, jawl::GamepadAxis::RX), 
            -jawl::controllerAxisValue(0, jawl::GamepadAxis::RY), 
            m_dt);
    }
    m_camera.updateView();

    m_angle+=20.f * m_dt;
    m_rotation = Matrix::multiply(Matrix::rotate(m_angle, 0), Matrix::rotate(m_angle, 1));
    m_rotation = Matrix::multiply(m_rotation, Matrix::rotate(m_angle, 2));
    
    m_model = Matrix::multiply(Matrix::multiply(m_translate, m_rotation), m_scale);
}

void Scene_Suzanne::render()
{
    m_renderer.clear(0x111111);

    Vec3f centre{ 0, 0, 0 };
    float radius = 1.f;
    
    if (m_camera.isSphereInsideFrustum(*shader.model, centre, radius))
    {
        // std::fill(Renderer::depthMap.begin(), Renderer::depthMap.end(), 1.f);

        // m_renderer.renderMode = Renderer::RenderMode::Shadow;
        // shader.isShadowPass = true;
        // m_renderer.draw(m_suzanne.vertices, shader);

        // m_renderer.renderMode = Renderer::RenderMode::Final;
        // shader.isShadowPass = false;
        m_renderer.draw(m_suzanne.vertices, shader);

        if (ssao)
        {
            m_ssao.apply();
        }
    }
    
    m_window.swapBuffers();
}

void Scene_Suzanne::shutdown()
{

}

void Scene_Suzanne::onWindowResize(int w, int h)
{
    m_renderer.viewport(0, 0, w, h);
}