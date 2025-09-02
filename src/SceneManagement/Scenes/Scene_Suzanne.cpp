#include "Scene_Suzanne.h"

#include <chrono>

Scene_Suzanne::Scene_Suzanne(Window& window) :
    m_window(window)
{
}

void Scene_Suzanne::start()
{
    for (int i = 0; i < m_suzanne.vertices.size(); i++)
    {
        m_suzanne.vertices[i].colour = Utils::pcgHash(rand());
    }

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

void Scene_Suzanne::processInput()
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

void Scene_Suzanne::update()
{
    m_angle+=0.2f;
    
    m_rotation = Matrix::multiply(Matrix::rotate(m_angle, 0), Matrix::rotate(m_angle, 1));
    m_rotation = Matrix::multiply(m_rotation, Matrix::rotate(m_angle, 2));

    m_model = Matrix::multiply(Matrix::multiply(m_translate, m_rotation), m_scale);
}

void Scene_Suzanne::render()
{
    m_window.clear(0x000000);

    m_window.draw(m_suzanne.vertices, &m_texture, m_model, m_view, m_projection);

    m_window.display();
}

void Scene_Suzanne::shutdown()
{

}