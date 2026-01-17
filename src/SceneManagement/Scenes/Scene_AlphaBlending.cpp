#include "Scene_AlphaBlending.h"

Scene_AlphaBlending::Scene_AlphaBlending(jawl::Window &window) : 
    m_window(window)
{
    int w, h;
    m_window.getSize(w, h);
    m_camera = Camera(45.f, (float)w / (float)h, 0.5f, 50.f);

    m_renderer = echo::Renderer(m_window.renderTarget());
    m_renderer.setVertexAttrib(sizeof(Vertex), 0, 4); /* Position */
    m_renderer.setVertexAttrib(sizeof(Vertex), 1, 3); /* Normal */
    m_renderer.setVertexAttrib(sizeof(Vertex), 2, 2); /* TexCoord */
    m_renderer.setVertexAttrib(sizeof(Vertex), 3, 3); /* Tangent */
    m_renderer.setVertexAttrib(sizeof(Vertex), 4, 3); /* Bitangent */

    int s = 2;
    m_renderer.internalDownScale(s);

    m_window.getScreenSize(w, h);

    m_rt.colour.resize((w/s)*(h/s));
    m_rt.depth.resize((w/s)*(h/s));
    m_rt.rect.x = 0;
    m_rt.rect.y = 0;
    m_rt.rect.width  = w/s;
    m_rt.rect.height = h/s;

    m_renderer.setRenderTarget(m_rt);
}

void Scene_AlphaBlending::start()
{
    m_grassShader.setUniform<echo::mat4>(m_grassShader.projection, m_camera.projection());
    m_grassShader.setUniform<echo::mat4>(m_grassShader.view, m_camera.view());
    m_grassShader.setUniform<echo::mat4>(m_grassShader.model, m_grassModel);

    m_cubeShader.setUniform<echo::mat4>(m_cubeShader.projection, m_camera.projection());
    m_cubeShader.setUniform<echo::mat4>(m_cubeShader.view, m_camera.view());
    m_cubeShader.setUniform<echo::mat4>(m_cubeShader.model, m_cubeModel);

    m_windowShader.setUniform<echo::mat4>(m_windowShader.projection, m_camera.projection());
    m_windowShader.setUniform<echo::mat4>(m_windowShader.view, m_camera.view());
    m_windowShader.setUniform<echo::mat4>(m_windowShader.model, m_windowModel);

    m_grassModel = echo::identity() * (echo::rotate(180, { 0, 1, 0 }) * echo::identity());
    m_windowModel = echo::translate({ 2.f, 0.f, -3.f }) * (echo::rotate(180, { 0, 1, 0 }) * echo::identity());

    stbi_set_flip_vertically_on_load(true);

    int iw, ih, ic;
    uint8_t *g = stbi_load("./resources/texture_grass.tga", &iw, &ih, &ic, 0);
    m_grassTexture = m_renderer.createTexture(g, iw, ih, ic);
    m_grassTexture.wrapMode = echo::WrapMode::MirroredRepeat;

    uint8_t *d = stbi_load("./resources/texture_dirtcube.tga", &iw, &ih, &ic, 0);
    m_dirtTexture = m_renderer.createTexture(d, iw, ih, ic);

    uint8_t *w = stbi_load("./resources/texture_window.tga", &iw, &ih, &ic, 0);
    m_windowTexture = m_renderer.createTexture(w, iw, ih, ic);

    stbi_image_free(g);
    stbi_image_free(d);
    stbi_image_free(w);

    // m_renderer.generateMipMaps(m_grassTexture);
    // m_renderer.generateMipMaps(m_dirtTexture);
    // m_renderer.generateMipMaps(m_windowTexture);

    m_grassShader.setUniform<echo::Texture>(m_grassShader.grassTexture, m_grassTexture);
    m_cubeShader.setUniform<echo::Texture>(m_cubeShader.cubeTexture, m_dirtTexture);
    m_windowShader.setUniform<echo::Texture>(m_windowShader.windowTexture, m_windowTexture);

    double previous = getTimeSeconds();
    while (m_window.isOpen())
    {
        double current = getTimeSeconds();
        m_dt = current-previous;
        previous = current;

        processInput();
        update();
        render();

        // printf("FPS: %.0f\n", 1.f/m_dt);
    }

    m_window.cleanup();
}

void Scene_AlphaBlending::processInput()
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
                    m_window.setFullscreen(fs = !fs);
                }
                
            } break;

            case jawl::Event::MouseMove:
            {
                m_camera.updateRotation(e.mousePos.x, e.mousePos.y);
            } break;
        }
    }
}

void Scene_AlphaBlending::update()
{
    m_camera.updateMovement(m_dt);
    m_camera.updateView();

    m_angle += 40.f * m_dt;
    m_cubeModel = echo::translate({ -7, 0, 7 }) * (echo::rotate(m_angle, { -1, 1, 1 }) * echo::identity());
}

void Scene_AlphaBlending::render()
{
    m_renderer.clear(0x111111);

    int w, h;
    m_window.getScreenSize(w, h);

    int scale = m_renderer.internalDownScale();
    int s = scale*2;

    m_renderer.viewport(w/s, h/s, w/s, h/s);
    m_renderer.bindVertexArray(m_cube.vertices.data(), m_cube.vertices.size());
    m_renderer.bindShader(m_cubeShader);
    m_renderer.drawArray(m_cube.vertices.data(), m_cube.vertices.size());

    m_renderer.viewport(0, 0, w/s, h/s);
    m_renderer.bindVertexArray(m_plane.vertices.data(), m_plane.vertices.size());
    m_renderer.bindShader(m_grassShader);
    m_renderer.drawArray(m_plane.vertices.data(), m_plane.vertices.size());

    m_renderer.bindShader(m_windowShader);
    m_renderer.drawArray(m_plane.vertices.data(), m_plane.vertices.size());

    m_renderer.endDraw();

    m_window.swapBuffers();
}

void Scene_AlphaBlending::shutdown()
{
    
}