#include "Scene_Suzanne.h"

#include <stb_image/stb_image.h>
#include <iostream>

Scene_Suzanne::Scene_Suzanne(jawl::Window &window) :
    m_window(window)
{
    int x = 0;
    int y = 0;
    m_window.getSize(x, y);
    m_camera = Camera(45.f, (float)x / (float)y, 0.5f, 20.f);

    m_renderer = echo::Renderer(m_window.getEBI());
    m_renderer.setVertexAttrib(sizeof(Vertex), 0, 4); /* Position */
    m_renderer.setVertexAttrib(sizeof(Vertex), 1, 3); /* Normal */
    m_renderer.setVertexAttrib(sizeof(Vertex), 2, 2); /* TexCoord */
    m_renderer.setVertexAttrib(sizeof(Vertex), 3, 3); /* Tangent */
    m_renderer.setVertexAttrib(sizeof(Vertex), 4, 3); /* Bitangent */
}

void Scene_Suzanne::start()
{
    shader.setUniform<echo::mat4>(shader.model, m_model);
    shader.setUniform<echo::mat4>(shader.view, m_camera.view());
    shader.setUniform<echo::mat4>(shader.projection, m_camera.projection());

    stbi_set_flip_vertically_on_load(true);

    int iw, ih, ic;
    uint8_t *t = stbi_load("./resources/suzanne_diffuse.jpg", &iw, &ih, &ic, 0);
    m_texture = m_renderer.createTexture(t, iw, ih, ic);

    uint8_t *n = stbi_load("./resources/suzanne_normal.jpg", &iw, &ih, &ic, 0);
    m_normalMap = m_renderer.createTexture(n, iw, ih, ic);

    uint8_t *s = stbi_load("./resources/suzanne_specular.jpg", &iw, &ih, &ic, 0);
    m_specularMap = m_renderer.createTexture(s, iw, ih, ic);

    m_renderer.generateMipMaps(m_texture);
    m_renderer.generateMipMaps(m_normalMap);
    m_renderer.generateMipMaps(m_specularMap);
    m_renderer.bilinearFilter(m_texture);
    m_renderer.bilinearFilter(m_normalMap);

    stbi_image_free(t);
    stbi_image_free(n);
    stbi_image_free(s);

    shader.setUniform<echo::Texture>(shader.diffuseMap, m_texture);
    shader.setUniform<echo::Texture>(shader.normalMap, m_normalMap);
    shader.setUniform<echo::Texture>(shader.specularMap, m_specularMap);

    uint16_t w, h;
    m_renderer.size(w, h);
    m_ssao = SSAO_Shader(m_renderer.framebuffer(), m_renderer.depthbuffer(), w, h);

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
    if (jawl::isGamepadConnected(0))
    {
        m_camera.updateRotation(
             jawl::controllerAxisValue(0, jawl::GamepadAxis::RX), 
            -jawl::controllerAxisValue(0, jawl::GamepadAxis::RY),
            m_dt);
    }
    m_camera.updateMovement(m_dt);
    m_camera.updateView();

    m_angle += 1.f * m_dt;
    m_model = echo::identity() * (echo::rotate(m_angle, { 0.5f, 1, 0 }) * echo::identity());
}

void Scene_Suzanne::render()
{
    m_renderer.clear(0x111111);

    echo::vec3f centre{ 0, 0, 0 };
    float radius = 1.f;
    
    if (m_camera.isSphereInsideFrustum(*shader.model, centre, radius))
    {
        // std::fill(Renderer::depthMap.begin(), Renderer::depthMap.end(), 1.f);

        // m_renderer.renderMode = Renderer::RenderMode::Shadow;
        // shader.isShadowPass = true;
        // m_renderer.draw(m_suzanne.vertices, shader);

        // m_renderer.renderMode = Renderer::RenderMode::Final;
        // shader.isShadowPass = false;

        m_renderer.draw(m_suzanne.vertices.data(), m_suzanne.vertices.size(), shader);

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