#include "Scene_AlphaBlending.h"

Scene_AlphaBlending::Scene_AlphaBlending(AppContext &app) : 
    m_app(app)
{
    u32 w = app.getWindow().width;
    u32 h = app.getWindow().height;
    m_camera = Camera(45.f, (f32)w / (f32)h, 0.5f, 20.f);

    app.getRenderer().setVertexAttrib(sizeof(Vertex), 0, 4); /* Position */
    app.getRenderer().setVertexAttrib(sizeof(Vertex), 1, 3); /* Normal */
    app.getRenderer().setVertexAttrib(sizeof(Vertex), 2, 2); /* TexCoord */
    app.getRenderer().setVertexAttrib(sizeof(Vertex), 3, 3); /* Tangent */
    app.getRenderer().setVertexAttrib(sizeof(Vertex), 4, 3); /* Bitangent */

    // int scale = 2;
    // app.getRenderer().internalDownScale(scale);
    // w/=scale;
    // h/=scale;

    // RenderTarget rt;
    // rt.colour.resize(w*h);
    // rt.depth.resize(w*h);
    // rt.rect = { 0, 0, (u16)w, (u16)h };

    // app.getRenderer().setRenderTarget(rt);
}

void Scene_AlphaBlending::start()
{
    m_grassShader.setUniform<mat4>(m_grassShader.projection, m_camera.projection());
    m_grassShader.setUniform<mat4>(m_grassShader.view, m_camera.view());
    m_grassShader.setUniform<mat4>(m_grassShader.model, m_grassModel);

    m_cubeShader.setUniform<mat4>(m_cubeShader.projection, m_camera.projection());
    m_cubeShader.setUniform<mat4>(m_cubeShader.view, m_camera.view());
    m_cubeShader.setUniform<mat4>(m_cubeShader.model, m_cubeModel);

    m_windowShader.setUniform<mat4>(m_windowShader.projection, m_camera.projection());
    m_windowShader.setUniform<mat4>(m_windowShader.view, m_camera.view());
    m_windowShader.setUniform<mat4>(m_windowShader.model, m_windowModel);

    m_grassModel = identity() * (rotate(180, { 0, 1, 0 }) * identity());
    m_windowModel = translate({ 2.f, 0.f, -3.f }) * (rotate(180, { 0, 1, 0 }) * identity());

    stbi_set_flip_vertically_on_load(true);

    int iw, ih, ic;
    uint8_t *g = stbi_load("./resources/texture_grass.tga", &iw, &ih, &ic, 0);
    m_grassTexture = m_app.getRenderer().createTexture(g, iw, ih, ic);
    m_grassTexture.wrapMode = WrapMode::MirroredRepeat;

    uint8_t *d = stbi_load("./resources/texture_dirtcube.tga", &iw, &ih, &ic, 0);
    m_dirtTexture = m_app.getRenderer().createTexture(d, iw, ih, ic);

    uint8_t *w = stbi_load("./resources/texture_window.tga", &iw, &ih, &ic, 0);
    m_windowTexture = m_app.getRenderer().createTexture(w, iw, ih, ic);

    stbi_image_free(g);
    stbi_image_free(d);
    stbi_image_free(w);

    // m_app.getRenderer().generateMipMaps(m_grassTexture);
    // m_app.getRenderer().generateMipMaps(m_dirtTexture);
    // m_app.getRenderer().generateMipMaps(m_windowTexture);

    m_grassShader.setUniform<Texture>(m_grassShader.grassTexture, m_grassTexture);
    m_cubeShader.setUniform<Texture>(m_cubeShader.cubeTexture, m_dirtTexture);
    m_windowShader.setUniform<Texture>(m_windowShader.windowTexture, m_windowTexture);

    timepoint_ previous = get_time();
    while (window_is_open(&m_app.getWindow()))
    {
        timepoint_ current = get_time();
        duration_ diff = timepoint_diff(&current, &previous);
        m_dt = (f32)diff.us / 1000000L;
        previous = current;

        processInput();
        update();
        render();

        window_sleep(&m_app.getWindow());

        printf("FPS: %.0f\n", 1.f/m_dt);
    }
}

void Scene_AlphaBlending::processInput()
{
    window_pump_messages(&m_app.getWindow());

    if (is_key_pressed(&m_app.getWindow().input, ESC))
    {
        shutdown();
    }

    m_camera.updateMovement(m_app.getWindow().input, m_dt);
    m_camera.updateRotation(m_app.getWindow().input);
    m_camera.updateView();
}

void Scene_AlphaBlending::update()
{
    m_angle += 40.f * m_dt;
    m_cubeModel = translate({ -7, 0, 7 }) * (rotate(m_angle, { -1, 1, 1 }) * identity());
}

void Scene_AlphaBlending::render()
{
    m_app.getRenderer().clear(0x111111);

    u32 w = m_app.getWindow().width;
    u32 h = m_app.getWindow().height;

    int scale = m_app.getRenderer().internalDownScale();
    int s = scale*2;

    m_app.getRenderer().viewport(w/s, h/s, w/s, h/s);
    m_app.getRenderer().bindVertexArray(m_cube.vertices.data(), m_cube.vertices.size());
    m_app.getRenderer().bindShader(m_cubeShader);
    m_app.getRenderer().drawArray(m_cube.vertices.data(), m_cube.vertices.size());

    m_app.getRenderer().viewport(0, 0, w/s, h/s);
    m_app.getRenderer().bindVertexArray(m_plane.vertices.data(), m_plane.vertices.size());
    m_app.getRenderer().bindShader(m_grassShader);
    m_app.getRenderer().drawArray(m_plane.vertices.data(), m_plane.vertices.size());

    m_app.getRenderer().bindShader(m_windowShader);
    m_app.getRenderer().drawArray(m_plane.vertices.data(), m_plane.vertices.size());

    m_app.getRenderer().endDraw();

    window_swap_buffers(&m_app.getWindow());
}

void Scene_AlphaBlending::shutdown()
{
    m_cubeShader.projection.release();
    m_cubeShader.view.release();
    m_cubeShader.model.release();
    m_cubeShader.cubeTexture.release();

    m_grassShader.projection.release();
    m_grassShader.view.release();
    m_grassShader.model.release();
    m_grassShader.grassTexture.release();

    m_windowShader.projection.release();
    m_windowShader.view.release();
    m_windowShader.model.release();
    m_windowShader.windowTexture.release();

    scene_manager::removeScene();
}