#include "Scene_Triangle.h"

Scene_Triangle::Scene_Triangle(jawl::Window &window) :
    m_window(window)
{
    int x = 0;
    int y = 0;
    m_window.getSize(x, y);
    m_camera = Camera(45.f, (float)x / (float)y, 0.5f, 20.f);
}

void Scene_Triangle::start()
{
    m_triangle.resize(3);
    m_triangle[0].position = { -1, 1, 0 };
    m_triangle[0].colour = 0xff0000;
    m_triangle[1].position = { 0, -1, 0 };
    m_triangle[1].colour = 0x00ff00;
    m_triangle[2].position = { 1, 1, 0 };
    m_triangle[2].colour = 0x0000ff;

    double previous = getTimeSeconds();
    while (m_window.isOpen())
    {
        double current = getTimeSeconds();
        m_dt = current-previous;
        previous = current;

        processInput();
        update();
        render();

        //printf("FPS: %.0f\n", 1.f/m_dt);
    }
}

void Scene_Triangle::processInput()
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

void Scene_Triangle::update()
{
    m_camera.updateMovement(m_dt);
    m_angle+=20.f * m_dt;
    
    m_rotation = Matrix::multiply(Matrix::rotate(m_angle, 0), Matrix::rotate(m_angle, 1));
    m_rotation = Matrix::multiply(m_rotation, Matrix::rotate(m_angle, 2));

    m_model = Matrix::multiply(Matrix::multiply(m_translate, m_rotation), m_scale);
    m_camera.updateView();
}

void Scene_Triangle::render()
{
    m_renderer.clear(0x111111);

    // m_renderer.draw(m_triangle, m_shader);

    m_window.swapBuffers();
}

void Scene_Triangle::shutdown()
{

}