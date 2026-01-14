#include "Scene_Suzanne.h"

Scene_Suzanne::Scene_Suzanne(jawl::Window &window) :
    m_window(window)
{
    int w, h;
    m_window.getSize(w, h);
    m_camera = Camera(45.f, (float)w / (float)h, 0.5f, 20.f);

    m_renderer = echo::Renderer(m_window.renderTarget());
    m_renderer.setVertexAttrib(sizeof(Vertex), 0, 4); /* Position */
    m_renderer.setVertexAttrib(sizeof(Vertex), 1, 3); /* Normal */
    m_renderer.setVertexAttrib(sizeof(Vertex), 2, 2); /* TexCoord */
    m_renderer.setVertexAttrib(sizeof(Vertex), 3, 3); /* Tangent */
    m_renderer.setVertexAttrib(sizeof(Vertex), 4, 3); /* Bitangent */

    int scale = 2;
    m_renderer.internalDownScale(scale);
    m_window.getScreenSize(w, h);
    w/=scale;
    h/=scale;

    echo::RenderTarget rt;
    rt.colour.resize(w*h);
    rt.depth.resize(w*h);
    rt.rect = { 0, 0, (uint16_t)w, (uint16_t)h };

    m_renderer.setRenderTarget(rt);
}

void Scene_Suzanne::start()
{
    m_shader.setUniform<echo::mat4>(m_shader.model, m_model);
    m_shader.setUniform<echo::mat4>(m_shader.view, m_camera.view());
    m_shader.setUniform<echo::mat4>(m_shader.projection, m_camera.projection());
    m_shader.setUniform<echo::Texture>(m_shader.diffuseMap, m_texture);
    m_shader.setUniform<echo::Texture>(m_shader.normalMap, m_normalMap);
    m_shader.setUniform<echo::Texture>(m_shader.specularMap, m_specularMap);

    stbi_set_flip_vertically_on_load(true);

    int iw, ih, ic;
    uint8_t *t = stbi_load("./resources/suzanne_diffuse.jpg", &iw, &ih, &ic, 0);
    m_texture = m_renderer.createTexture(t, iw, ih, ic);

    uint8_t *n = stbi_load("./resources/suzanne_normal.jpg", &iw, &ih, &ic, 0);
    m_normalMap = m_renderer.createTexture(n, iw, ih, ic);

    uint8_t *s = stbi_load("./resources/suzanne_specular.jpg", &iw, &ih, &ic, 0);
    m_specularMap = m_renderer.createTexture(s, iw, ih, ic);

    m_renderer.generateMipMaps(m_texture);
    // m_renderer.generateMipMaps(m_normalMap);
    // m_renderer.generateMipMaps(m_specularMap);
    m_renderer.bilinearFilter(m_texture);
    m_renderer.bilinearFilter(m_normalMap);

    stbi_image_free(t);
    stbi_image_free(n);
    stbi_image_free(s);

    m_renderer.bindShader(m_shader);

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
}

void Scene_Suzanne::processInput()
{
    jawl::Event e;
    while (m_window.pollEvents(e))
    {
        switch(e.type)
        {
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
                m_camera.updateRotation(e.mousePos.x, e.mousePos.y);
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
            } break;

            default: break;
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

    m_angle += 40.f * m_dt;
    m_model = echo::identity() * (echo::rotate(m_angle, { 0.5f, 1, 1 }) * echo::identity());
}

void Scene_Suzanne::render()
{
    m_renderer.clear(0x111111);

    echo::vec3f centre{ 0, 0, 0 };
    float radius = 1.f;
    
    if (m_camera.isSphereInsideFrustum(*m_shader.model, centre, radius))
    {
        m_renderer.drawArray(m_suzanne.vertices.data(), m_suzanne.vertices.size());

        if (ssao)
        {
            echo::RenderTarget &rt = m_renderer.rendertarget();
            m_ssao.initAOBuffers(rt.rect.width, rt.rect.height);
            m_ssao.apply(rt);
        }
    }

    m_renderer.endDraw();
    
    m_window.swapBuffers();
}

void Scene_Suzanne::shutdown()
{

}