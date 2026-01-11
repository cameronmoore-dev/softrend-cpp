#include "Scene_Assimp.h"

#include <stb_image/stb_image.h>

Scene_Assimp::Scene_Assimp(jawl::Window &window) : 
    m_window(window)
{
    int w, h;
    m_window.getSize(w, h);
    m_camera = Camera(45.f, (float)w / (float)h, 0.5f, 20.f);

    m_renderer = echo::Renderer(m_window.renderTarget());
    m_renderer.setVertexAttrib(sizeof(AssimpVertex), 0, 4); /* Position */
    m_renderer.setVertexAttrib(sizeof(AssimpVertex), 1, 3); /* Normals */
    m_renderer.setVertexAttrib(sizeof(AssimpVertex), 2, 2); /* TexCoords */
}

void Scene_Assimp::start()
{
    m_shader.setUniform<echo::mat4>(m_shader.model, m_modelMat);
    m_shader.setUniform<echo::mat4>(m_shader.view, m_camera.view());
    m_shader.setUniform<echo::mat4>(m_shader.projection, m_camera.projection());

    m_modelMat = echo::identity() * (echo::rotate(180, { 0, 1, 0 }) * echo::scale({ 0.03f, 0.03f, 0.03f }));
    
    m_model.loadModel("./resources/male_character_ps1-style/scene.gltf");

    int w, h, c;
    uint8_t *tex = stbi_load("./resources/hero_baseColor.png", &w, &h, &c, 0);
    m_texture = m_renderer.createTexture(tex, w, h, c);

    stbi_image_free(tex);

    m_shader.setUniform<echo::Texture>(m_shader.diffuse, m_texture);

    AssimpMesh &mesh = m_model.meshes[0];
    m_renderer.bindVertexArray(mesh.m_vertices.data(), mesh.m_vertices.size());
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
            } break;

            case jawl::Event::MouseMove:
            {
                m_camera.updateRotation(event.mousePos.x, event.mousePos.y, m_dt);
            } break;
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
    m_renderer.clear(0x111111);

    AssimpMesh &mesh = m_model.meshes[0];
    m_renderer.drawIndex(mesh.m_indices);
    m_renderer.endDraw();

    m_window.swapBuffers();
}

void Scene_Assimp::shutdown()
{

}