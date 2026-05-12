#include "Scene_Cube.h"

Scene_Cube::Scene_Cube(jawl::Window &window) :
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

    m_renderer.bindVertexArray(m_cube.vertices.data(), m_cube.vertices.size());
    m_renderer.bindShader(m_shader);
}

void Scene_Cube::start()
{
    m_shader.setUniform<echo::mat4>(m_shader.model, m_model);
    m_shader.setUniform<echo::mat4>(m_shader.view, m_camera.view());
    m_shader.setUniform<echo::mat4>(m_shader.projection, m_camera.projection());

    stbi_set_flip_vertically_on_load(true);

    int iw, ih, ic;
    uint8_t *d = stbi_load("./resources/texture_dirtcube.tga", &iw, &ih, &ic, 0);
    m_texture = m_renderer.createTexture(d, iw, ih, ic);

    // m_renderer.bilinearFilter(m_texture);

    stbi_image_free(d);

    m_shader.setUniform<echo::Texture>(m_shader.cubeTexture, m_texture);
    
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

void Scene_Cube::processInput()
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
                    shutdown();
                }

                else if (e.keyCode == jawl::getKey(jawl::Key::F))
                {
                    doFXAA = !doFXAA;
                }
            } break;

            case jawl::Event::KeyReleased:
            {
            } break;

            case jawl::Event::MouseMove:
            {
                m_camera.updateRotation(e.mousePos.x, e.mousePos.y);
            } break;
        }
    }
}

void Scene_Cube::update()
{
    m_camera.updateMovement(m_dt);
    m_camera.updateView();

    m_angle += 40.f * m_dt;
    m_model = echo::identity() * (echo::rotate(m_angle, {-1, 1, 2 }) * echo::identity());
}

void Scene_Cube::render()
{
    m_renderer.clear(0x111111);

    m_renderer.drawArray(m_cube.vertices.data(), m_cube.vertices.size());

    if (doFXAA)
    {
        m_fxaa.apply(m_renderer.rendertarget());
    }
    
    m_renderer.endDraw();

    m_window.swapBuffers();
}

void Scene_Cube::shutdown()
{
    m_shader.projection.release();
    m_shader.view.release();
    m_shader.model.release();
    m_shader.cubeTexture.release();

    scene_manager::removeScene();
}