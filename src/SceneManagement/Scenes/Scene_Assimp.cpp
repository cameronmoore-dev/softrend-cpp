#include "Scene_Assimp.h"

Scene_Assimp::Scene_Assimp(AppContext &app) : 
    m_app(app)
{
    u32 w = app.getWindow().width;
    u32 h = app.getWindow().height;
    m_camera = Camera(45.f, (f32)w / (f32)h, 0.5f, 20.f);

    m_lightSource = Camera(-10, 10, -10, 10, 1.f, 20.f);
    m_lightSource.m_position = { 3.221032, 4.042738, -3.244524 };
    m_lightSource.m_forward  = { 0.557615, 0.618408, -0.553748 };
    m_lightSource.m_yaw   = -44.800663;
    m_lightSource.m_pitch = 38.199978;
    m_lightSource.updateView();
    m_shader.lightDir = vec3f{ 0.f, 0.f, 0.f } - m_lightSource.position();

    mat4 lsm = m_lightSource.projection() * m_lightSource.view();
    m_dpthshdr.lightSpaceMatrix = lsm;
    m_shader.lightSpaceMatrix = lsm;

    m_app.getRenderer().setVertexAttrib(sizeof(AssimpVertex), 0, 4); /* Position */
    m_app.getRenderer().setVertexAttrib(sizeof(AssimpVertex), 1, 3); /* Normals */
    m_app.getRenderer().setVertexAttrib(sizeof(AssimpVertex), 2, 2); /* TexCoords */
    m_app.getRenderer().setVertexAttrib(sizeof(AssimpVertex), 3, 3); /* FragPos */

    uint16_t depthMapSize = 512;
    m_depthMap.depth.resize(depthMapSize*depthMapSize);
    m_depthMap.rect = { 0, 0, depthMapSize, depthMapSize };

    m_shader.shadowMapSize = depthMapSize;

    // int scale = 2;
    // app.getRenderer().internalDownScale(scale);
    // w/=scale;
    // h/=scale;

    m_default.colour.resize(w*h);
    m_default.depth.resize(w*h);
    m_default.rect = { 0, 0, (u16)w, (u16)h };
}

void Scene_Assimp::start()
{
    m_shader.setUniform<mat4>(m_shader.model, m_modelMat);
    m_shader.setUniform<mat4>(m_shader.view, m_camera.view());
    m_shader.setUniform<mat4>(m_shader.projection, m_camera.projection());
    
    m_model.loadModel("./resources/male_character_ps1-style/scene.gltf");
    m_grnd.loadModel("./resources/model_plane.obj");

    stbi_set_flip_vertically_on_load(false);
    int w, h, c;
    uint8_t *tex = stbi_load("./resources/hero_baseColor.png", &w, &h, &c, 0);
    m_texture = m_app.getRenderer().createTexture(tex, w, h, c);

    stbi_image_free(tex);

    m_shader.setUniform<Texture>(m_shader.diffuse, m_texture);

    /* Shadow Pass */
    // m_app.getRenderer().setRenderTarget(m_depthMap);
    // m_app.getRenderer().bindShader(m_dpthshdr);
    // m_app.getRenderer().clear(0x111111);

    // m_modelMat = translate({ 0, -3, 0 }) * (rotate(-90, { 1, 0, 0 }) * scale({ 5, 5, 5 }));
    // m_dpthshdr.model = m_modelMat;
    // m_app.getRenderer().bindVertexArray(m_grnd.meshes[0].m_vertices.data(), m_grnd.meshes[0].m_vertices.size());
    // m_app.getRenderer().drawIndex(m_grnd.meshes[0].m_indices);

    // m_modelMat = translate({ 0, -3, 0 }) * (rotate(180, { 0, 1, 0 }) * scale({ 0.03f, 0.03f, 0.03f }));
    // m_dpthshdr.model = m_modelMat;
    // m_app.getRenderer().bindVertexArray(m_model.meshes[0].m_vertices.data(), m_model.meshes[0].m_vertices.size());
    // m_app.getRenderer().drawIndex(m_model.meshes[0].m_indices);
    
    // m_shader.shadowMap = m_app.getRenderer().rendertarget().depth;
    /* -- */

    m_app.getRenderer().setRenderTarget(m_default);
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

void Scene_Assimp::processInput()
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

void Scene_Assimp::update()
{
}

void Scene_Assimp::render()
{
    // m_app.getRenderer().clear(0x111111);

    angle += 20.f * m_dt;
    
    /* Shadow Pass */
    m_app.getRenderer().setRenderTarget(m_depthMap);
    m_app.getRenderer().bindShader(m_dpthshdr);
    m_app.getRenderer().clear(0x111111);

    // Floor model
    m_modelMat = translate({ 0, -3, 0 }) * (rotate(-90, { 1, 0, 0 }) * scale({ 5, 5, 5 }));
    m_dpthshdr.model = m_modelMat;
    m_app.getRenderer().bindVertexArray(m_grnd.meshes[0].m_vertices.data(), m_grnd.meshes[0].m_vertices.size());
    m_app.getRenderer().drawIndex(m_grnd.meshes[0].m_indices);

    // Character model
    m_modelMat = translate({ 0, -3, 0 }) * (rotate(angle, { 0, 1, 0 }) * scale({ 0.03f, 0.03f, 0.03f }));
    m_dpthshdr.model = m_modelMat;
    m_app.getRenderer().bindVertexArray(m_model.meshes[0].m_vertices.data(), m_model.meshes[0].m_vertices.size());
    m_app.getRenderer().drawIndex(m_model.meshes[0].m_indices);
    
    m_shader.shadowMap = m_app.getRenderer().rendertarget().depth;
    /* -- */

    m_app.getRenderer().setRenderTarget(m_default);
    m_app.getRenderer().bindShader(m_shader);
    m_app.getRenderer().clear(0x111111);

    m_modelMat = translate({ 0, -3, 0 }) * (rotate(-90, { 1, 0, 0 }) * scale({ 5, 5, 5 }));
    m_app.getRenderer().bindVertexArray(m_grnd.meshes[0].m_vertices.data(), m_grnd.meshes[0].m_vertices.size());
    m_app.getRenderer().drawIndex(m_grnd.meshes[0].m_indices);

    m_modelMat = translate({ 0, -3, 0 }) * (rotate(angle, { 0, 1, 0 }) * scale({ 0.03f, 0.03f, 0.03f }));
    m_app.getRenderer().bindVertexArray(m_model.meshes[0].m_vertices.data(), m_model.meshes[0].m_vertices.size());
    m_app.getRenderer().drawIndex(m_model.meshes[0].m_indices);

    m_app.getRenderer().endDraw();

    window_swap_buffers(&m_app.getWindow());
}

void Scene_Assimp::shutdown()
{
    m_shader.projection.release();
    m_shader.view.release();
    m_shader.model.release();
    m_shader.diffuse.release();

    scene_manager::removeScene();
}