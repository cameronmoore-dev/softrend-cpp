#include "Scene_Suzanne.h"

Scene_Suzanne::Scene_Suzanne(AppContext &app) : 
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

    // app.getRenderer().setRenderTarget(rt);
}

void Scene_Suzanne::start()
{
    m_shader.setUniform<mat4>(m_shader.model, m_model);
    m_shader.setUniform<mat4>(m_shader.view, m_camera.view());
    m_shader.setUniform<mat4>(m_shader.projection, m_camera.projection());
    m_shader.setUniform<Texture>(m_shader.diffuseMap, m_texture);
    m_shader.setUniform<Texture>(m_shader.normalMap, m_normalMap);
    m_shader.setUniform<Texture>(m_shader.specularMap, m_specularMap);

    stbi_set_flip_vertically_on_load(true);

    int iw, ih, ic;
    uint8_t *t = stbi_load("./resources/suzanne_diffuse.jpg", &iw, &ih, &ic, 0);
    m_texture = m_app.getRenderer().createTexture(t, iw, ih, ic);

    uint8_t *n = stbi_load("./resources/suzanne_normal.jpg", &iw, &ih, &ic, 0);
    m_normalMap = m_app.getRenderer().createTexture(n, iw, ih, ic);

    uint8_t *s = stbi_load("./resources/suzanne_specular.jpg", &iw, &ih, &ic, 0);
    m_specularMap = m_app.getRenderer().createTexture(s, iw, ih, ic);

    m_app.getRenderer().generateMipMaps(m_texture);
    m_app.getRenderer().generateMipMaps(m_normalMap);
    m_app.getRenderer().generateMipMaps(m_specularMap);
    m_app.getRenderer().bilinearFilter(m_texture);
    m_app.getRenderer().bilinearFilter(m_normalMap);

    stbi_image_free(t);
    stbi_image_free(n);
    stbi_image_free(s);

    m_app.getRenderer().bindShader(m_shader);

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

void Scene_Suzanne::processInput()
{
    window_pump_messages(&m_app.getWindow());

    if (is_key_pressed(&m_app.getWindow().input, ESC))
    {
        shutdown();
    }

    else if (is_key_pressed(&m_app.getWindow().input, NUM1))
    {
        ssao = !ssao;
    }

    m_camera.updateMovement(m_app.getWindow().input, m_dt);
    m_camera.updateRotation(m_app.getWindow().input);
    m_camera.updateView();
}

void Scene_Suzanne::update()
{
    m_angle += 40.f * m_dt;
    m_model = identity() * (rotate(m_angle, { 0.5f, 1, 1 }) * identity());
}

void Scene_Suzanne::render()
{
    m_app.getRenderer().clear(0x111111);

    vec3f centre{ 0, 0, 0 };
    float radius = 1.f;
    
    if (m_camera.isSphereInsideFrustum(*m_shader.model, centre, radius))
    {
        m_app.getRenderer().drawArray(m_suzanne.vertices.data(), m_suzanne.vertices.size());

        if (ssao)
        {
            RenderTarget &rt = m_app.getRenderer().rendertarget();
            m_ssao.initAOBuffers(rt.rect.width, rt.rect.height);
            m_ssao.apply(rt);
        }
    }

    m_app.getRenderer().endDraw();
    
    window_swap_buffers(&m_app.getWindow());
}

void Scene_Suzanne::shutdown()
{
    m_shader.projection.release();
    m_shader.view.release();
    m_shader.model.release();

    m_shader.diffuseMap.release();
    m_shader.specularMap.release();
    m_shader.normalMap.release();

    scene_manager::removeScene();
}