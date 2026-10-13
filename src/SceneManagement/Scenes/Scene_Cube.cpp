#include "Scene_Cube.h"

Scene_Cube::Scene_Cube(AppContext &app) :
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
    
    int scale = 2;
    app.getRenderer().internalDownScale(scale);
    w/=scale;
    h/=scale;

    RenderTarget rt;
    rt.colour.resize(w*h);
    rt.depth.resize(w*h);
    rt.rect = { 0, 0, (u16)w, (u16)h };

    app.getRenderer().setRenderTarget(rt);

    app.getRenderer().bindVertexArray(m_cube.vertices.data(), m_cube.vertices.size());
    app.getRenderer().bindShader(m_shader);
}

void Scene_Cube::start()
{
    m_shader.setUniform<mat4>(m_shader.model, m_model);
    m_shader.setUniform<mat4>(m_shader.view, m_camera.view());
    m_shader.setUniform<mat4>(m_shader.projection, m_camera.projection());

    stbi_set_flip_vertically_on_load(true);

    int iw, ih, ic;
    uint8_t *d = stbi_load("./resources/texture_dirtcube.tga", &iw, &ih, &ic, 0);
    m_texture = m_app.getRenderer().createTexture(d, iw, ih, ic);

    // m_app.getRenderer().bilinearFilter(m_texture);

    stbi_image_free(d);

    m_shader.setUniform<Texture>(m_shader.cubeTexture, m_texture);
    
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

void Scene_Cube::processInput()
{
    window_pump_messages(&m_app.getWindow());

    if (is_key_pressed(&m_app.getWindow().input, ESC))
    {
        shutdown();
    }

    else if (is_key_pressed(&m_app.getWindow().input, NUM1))
    {
        doFXAA = !doFXAA;
    }

    m_camera.updateMovement(m_app.getWindow().input, m_dt);
    m_camera.updateRotation(m_app.getWindow().input);
    m_camera.updateView();
}

void Scene_Cube::update()
{
    m_angle += 40.f * m_dt;
    m_model = identity() * (rotate(m_angle, {-1, 1, 2 }) * identity());
}

void Scene_Cube::render()
{
    m_app.getRenderer().clear(0x111111);
    m_app.getRenderer().drawArray(m_cube.vertices.data(), m_cube.vertices.size());

    if (doFXAA)
    {
        m_fxaa.apply(m_app.getRenderer().rendertarget());
    }
    
    m_app.getRenderer().endDraw();

    window_swap_buffers(&m_app.getWindow());
}

void Scene_Cube::shutdown()
{
    m_shader.projection.release();
    m_shader.view.release();
    m_shader.model.release();
    m_shader.cubeTexture.release();

    scene_manager::removeScene();
}