#include "Scene_Cube.h"

#include <chrono>

Scene_Cube::Scene_Cube(jawl::Window &window) :
    m_window(window)
{
    int x = 0;
    int y = 0;
    m_window.getSize(x, y);
    m_camera = Camera(45.f, (float)x / (float)y, 0.5f, 20.f);

    m_renderer = echo::Renderer(m_window.getEBI());
}

void Scene_Cube::start()
{
    uint16_t w, h;
    m_renderer.size(w, h);
    m_fxaa = FXAA_Shader(m_renderer.framebuffer(), w*h, w);

    m_shader.setUniform<Mat4>(m_shader.model, m_model);
    m_shader.setUniform<Mat4>(m_shader.view, m_camera.view());
    m_shader.setUniform<Mat4>(m_shader.projection, m_camera.projection());
    // m_shader.setUniform<Image>(m_shader.texture, m_texture);
    
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
                    m_window.close();
                }
            } break;

            case jawl::Event::KeyReleased:
            {
            } break;

            case jawl::Event::MouseMove:
            {
                m_camera.updateRotation(e.mousePos.x, e.mousePos.y, m_dt);
            } break;
        }
    }
}

void Scene_Cube::update()
{
    m_camera.updateMovement(m_dt);
    m_camera.updateView();

    m_angle+=5.f * m_dt;
    m_rotation = Matrix::multiply(Matrix::rotate(m_angle, 0), Matrix::rotate(m_angle, 1));
    m_rotation = Matrix::multiply(m_rotation, Matrix::rotate(m_angle, 2));

    m_model = Matrix::multiply(Matrix::multiply(m_translate, m_rotation), m_scale);
}

void Scene_Cube::render()
{
    m_renderer.clear(0x111111);

    // m_renderer.draw(m_cube.vertices, m_shader);

    m_fxaa.apply();
    
    m_window.swapBuffers();
}

void Scene_Cube::shutdown()
{

}