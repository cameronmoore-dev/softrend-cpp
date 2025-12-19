#include "Scene_Cube.h"

#include <chrono>

Scene_Cube::Scene_Cube(Window& window) :
    m_window(window)
{
    int x = 0;
    int y = 0;
    m_window.getSize(x, y);
    m_camera = Camera(45.f, (float)x / (float)y, 0.5f, 20.f);
}

void Scene_Cube::start()
{
    m_shader.setUniform<Mat4>(m_shader.model, m_model);
    m_shader.setUniform<Mat4>(m_shader.view, m_camera.view());
    m_shader.setUniform<Mat4>(m_shader.projection, m_camera.projection());
    m_shader.setUniform<Image>(m_shader.texture, m_texture);
    
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
    Event e;
    while (m_window.pollEvents(e))
    {
        switch(e.type)
        {
            case Event::KeyPressed:
            {
                if (e.keyCode == VK_ESCAPE)
                {
                    m_window.close();
                }
            } break;

            case Event::KeyReleased:
            {
            } break;

            case Event::MouseMove:
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
    m_window.clear(0x111111);

    m_renderer.draw(m_cube.vertices, m_shader);

    m_window.display();
}

void Scene_Cube::shutdown()
{

}