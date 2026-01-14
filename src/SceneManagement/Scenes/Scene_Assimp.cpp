#include "Scene_Assimp.h"

Scene_Assimp::Scene_Assimp(jawl::Window &window) : 
    m_window(window)
{
    int w, h;
    m_window.getSize(w, h);
    m_camera = Camera(45.f, (float)w / (float)h, 0.5f, 20.f);

    m_lightSource = Camera(-10, 10, -10, 10, 1.f, 20.f);
    m_lightSource.m_position = { 3.221032, 4.042738, -3.244524 };
    m_lightSource.m_forward  = { 0.557615, 0.618408, -0.553748 };
    m_lightSource.m_yaw   = -44.800663;
    m_lightSource.m_pitch = 38.199978;
    m_lightSource.updateView();
    m_shader.lightDir = echo::vec3f{ 0.f, 0.f, 0.f } - m_lightSource.position();

    echo::mat4 lsm = m_lightSource.projection() * m_lightSource.view();
    m_dpthshdr.lightSpaceMatrix = lsm;
    m_shader.lightSpaceMatrix = lsm;

    m_renderer = echo::Renderer(m_window.renderTarget());
    m_renderer.setVertexAttrib(sizeof(AssimpVertex), 0, 4); /* Position */
    m_renderer.setVertexAttrib(sizeof(AssimpVertex), 1, 3); /* Normals */
    m_renderer.setVertexAttrib(sizeof(AssimpVertex), 2, 2); /* TexCoords */
    m_renderer.setVertexAttrib(sizeof(AssimpVertex), 3, 3); /* FragPos */

    uint16_t depthMapSize = 512;
    m_depthMap.depth.resize(depthMapSize*depthMapSize);
    m_depthMap.rect = { 0, 0, depthMapSize, depthMapSize };

    m_shader.shadowMapSize = depthMapSize;

    int resScale = 1;
    m_renderer.internalDownScale(resScale);
    m_window.getScreenSize(w, h);

    w /= resScale;
    h /= resScale;

    m_default.colour.resize(w*h);
    m_default.depth.resize(w*h);
    m_default.rect = { 0, 0, (uint16_t)w, (uint16_t)h };
}

void Scene_Assimp::start()
{
    m_shader.setUniform<echo::mat4>(m_shader.model, m_modelMat);
    m_shader.setUniform<echo::mat4>(m_shader.view, m_camera.view());
    m_shader.setUniform<echo::mat4>(m_shader.projection, m_camera.projection());
    
    m_model.loadModel("./resources/male_character_ps1-style/scene.gltf");
    m_grnd.loadModel("./resources/model_plane.obj");

    int w, h, c;
    uint8_t *tex = stbi_load("./resources/hero_baseColor.png", &w, &h, &c, 0);
    m_texture = m_renderer.createTexture(tex, w, h, c);

    stbi_image_free(tex);

    m_shader.setUniform<echo::Texture>(m_shader.diffuse, m_texture);

    /* Shadow Pass */
    // m_renderer.setRenderTarget(m_depthMap);
    // m_renderer.bindShader(m_dpthshdr);
    // m_renderer.clear(0x111111);

    // m_modelMat = echo::translate({ 0, -3, 0 }) * (echo::rotate(-90, { 1, 0, 0 }) * echo::scale({ 5, 5, 5 }));
    // m_dpthshdr.model = m_modelMat;
    // m_renderer.bindVertexArray(m_grnd.meshes[0].m_vertices.data(), m_grnd.meshes[0].m_vertices.size());
    // m_renderer.drawIndex(m_grnd.meshes[0].m_indices);

    // m_modelMat = echo::translate({ 0, -3, 0 }) * (echo::rotate(180, { 0, 1, 0 }) * echo::scale({ 0.03f, 0.03f, 0.03f }));
    // m_dpthshdr.model = m_modelMat;
    // m_renderer.bindVertexArray(m_model.meshes[0].m_vertices.data(), m_model.meshes[0].m_vertices.size());
    // m_renderer.drawIndex(m_model.meshes[0].m_indices);
    
    // m_shader.shadowMap = m_renderer.rendertarget().depth;
    /* -- */

    m_renderer.setRenderTarget(m_default);
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

        // printf("FPS: %.0f\n", 1.f/m_dt);
    }
}

void Scene_Assimp::processInput()
{
    jawl::Event event;
    while (m_window.pollEvents(event))
    {
        switch (event.type)
        {
            case jawl::Event::KeyPressed:
            {
                if (event.keyCode == jawl::getKey(jawl::Key::Escape))
                {
                    m_window.close();
                }

                else if (event.keyCode == jawl::getKey(jawl::Key::R))
                {
                    printf("pos: %f, %f, %f\n", m_camera.position().x, m_camera.position().y, m_camera.position().z);
                    printf("fwd: %f, %f, %f\n", m_camera.forward().x, m_camera.forward().y, m_camera.forward().z);
                    printf("angle: %f, %f\n", m_camera.m_yaw, m_camera.m_pitch);
                }
            } break;

            case jawl::Event::MouseMove:
            {
                m_camera.updateRotation(event.mousePos.x, event.mousePos.y);
            } break;

            default: break;
        }
    }
}

void Scene_Assimp::update()
{
    m_camera.updateMovement(m_dt);
    m_camera.updateView();
}

void Scene_Assimp::render()
{
    // m_renderer.clear(0x111111);

    angle += 20.f * m_dt;
    
    /* Shadow Pass */
    m_renderer.setRenderTarget(m_depthMap);
    m_renderer.bindShader(m_dpthshdr);
    m_renderer.clear(0x111111);

    m_modelMat = echo::translate({ 0, -3, 0 }) * (echo::rotate(-90, { 1, 0, 0 }) * echo::scale({ 5, 5, 5 }));
    m_dpthshdr.model = m_modelMat;
    m_renderer.bindVertexArray(m_grnd.meshes[0].m_vertices.data(), m_grnd.meshes[0].m_vertices.size());
    m_renderer.drawIndex(m_grnd.meshes[0].m_indices);

    m_modelMat = echo::translate({ 0, -3, 0 }) * (echo::rotate(angle, { 0, 1, 0 }) * echo::scale({ 0.03f, 0.03f, 0.03f }));
    m_dpthshdr.model = m_modelMat;
    m_renderer.bindVertexArray(m_model.meshes[0].m_vertices.data(), m_model.meshes[0].m_vertices.size());
    m_renderer.drawIndex(m_model.meshes[0].m_indices);
    
    m_shader.shadowMap = m_renderer.rendertarget().depth;
    /* -- */

    m_renderer.setRenderTarget(m_default);
    m_renderer.bindShader(m_shader);
    m_renderer.clear(0x111111);

    m_modelMat = echo::translate({ 0, -3, 0 }) * (echo::rotate(-90, { 1, 0, 0 }) * echo::scale({ 5, 5, 5 }));
    m_renderer.bindVertexArray(m_grnd.meshes[0].m_vertices.data(), m_grnd.meshes[0].m_vertices.size());
    m_renderer.drawIndex(m_grnd.meshes[0].m_indices);

    m_modelMat = echo::translate({ 0, -3, 0 }) * (echo::rotate(angle, { 0, 1, 0 }) * echo::scale({ 0.03f, 0.03f, 0.03f }));
    m_renderer.bindVertexArray(m_model.meshes[0].m_vertices.data(), m_model.meshes[0].m_vertices.size());
    m_renderer.drawIndex(m_model.meshes[0].m_indices);

    m_renderer.endDraw();

    m_window.swapBuffers();
}

void Scene_Assimp::shutdown()
{

}