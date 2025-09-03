#include "Scene_Triangle.h"

#include <chrono>

Scene_Triangle::Scene_Triangle(Window& window) :
    m_window(window)
{
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

    while (m_window.isOpen())
    {
        auto start = std::chrono::high_resolution_clock::now();

        m_window.pollMessages();
        processInput();
        update();
        render();  

        auto end = std::chrono::high_resolution_clock::now();
        double duration = std::chrono::duration<double>(end-start).count();

        printf("FPS: %.2f\n", 1.f/duration);
    }
}

void Scene_Triangle::processInput()
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
        }
    }
}

void Scene_Triangle::update()
{
    m_angle+=0.1f;
    
    m_rotation = Matrix::multiply(Matrix::rotate(m_angle, 2), Matrix::rotate(m_angle, 1));
    m_rotation = Matrix::multiply(m_rotation, Matrix::rotate(m_angle, 0));

    m_model = Matrix::multiply(Matrix::multiply(m_translate, m_rotation), m_scale);
}

void Scene_Triangle::render()
{
    m_window.clear(0x000000);

    m_window.draw(m_triangle, nullptr, m_model, m_view, m_projection);

    m_window.display();
}

void Scene_Triangle::shutdown()
{

}