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

                /* A */
                if (e.keyCode == 0x41)
                {
                    m_camera.isMovingLeft = true;
                }

                /* D */
                else if (e.keyCode == 0x44)
                {
                    m_camera.isMovingRight = true;
                }

                /* S */
                if (e.keyCode == 0x53)
                {
                    m_camera.isMovingBackward = true;
                }

                /* W */
                else if (e.keyCode == 0x57)
                {
                    m_camera.isMovingForward = true;
                }

                /* E */
                if (e.keyCode == 0x45)
                {
                    m_camera.isMovingUp = true;
                }

                /* Q */
                else if (e.keyCode == 0x51)
                {
                    m_camera.isMovingDown = true;
                }
            } break;

            case Event::KeyReleased:
            {
                /* A */
                if (e.keyCode == 0x41)
                {
                    m_camera.isMovingLeft = false;
                }

                /* D */
                else if (e.keyCode == 0x44)
                {
                    m_camera.isMovingRight = false;
                }

                /* S */
                if (e.keyCode == 0x53)
                {
                    m_camera.isMovingBackward = false;
                }

                /* W */
                else if (e.keyCode == 0x57)
                {
                    m_camera.isMovingForward = false;
                }

                /* E */
                if (e.keyCode == 0x45)
                {
                    m_camera.isMovingUp = false;
                }

                /* Q */
                else if (e.keyCode == 0x51)
                {
                    m_camera.isMovingDown = false;
                }
            } break;

            case Event::MouseMove:
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
    m_window.clear(0x000000);

    m_renderer.draw(m_triangle, nullptr, m_camera, m_model);

    m_window.display();
}

void Scene_Triangle::shutdown()
{

}