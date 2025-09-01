#include "Scene_Cube.h"

#include <chrono>

Scene_Cube::Scene_Cube(Window& window) :
    m_window(window)
{
}

void Scene_Cube::start()
{
    while (m_window.isOpen())
    {
        auto start = std::chrono::high_resolution_clock::now();

        m_window.pollMessages();
        processInput();
        update();
        render();  

        auto end = std::chrono::high_resolution_clock::now();
        double duration = std::chrono::duration<double>(end-start).count();

        printf("FPS: %f\n", 1/duration);
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
        }
    }
}

void Scene_Cube::update()
{
    angle+=0.1f;
    Mat4 translate = Matrix::identity();
    Mat4 scale = Matrix::identity();
    Mat4 rotation = Matrix::multiply(Matrix::rotate(Utils::degToRad(angle), 0), Matrix::rotate(Utils::degToRad(angle), 1));
    rotation = Matrix::multiply(rotation, Matrix::rotate(Utils::degToRad(angle), 2));

    m_model = Matrix::multiply(Matrix::multiply(translate, rotation), scale);
}

void Scene_Cube::render()
{
    m_window.clear(0x000000);

    m_window.draw(m_cube.vertices, m_texture, m_model, m_view, m_projection);

    m_window.display();
}

void Scene_Cube::shutdown()
{

}