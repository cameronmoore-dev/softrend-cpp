#include "Scene_Suzanne.h"

#include <chrono>

Scene_Suzanne::Scene_Suzanne(Window& window) :
    m_window(window)
{
}

void Scene_Suzanne::start()
{
    m_camera.updateRotation(0, 0);

    while (m_window.isOpen())
    {
        auto start = std::chrono::high_resolution_clock::now();

        m_window.pollMessages();
        processInput();
        update();
        render();  

        auto end = std::chrono::high_resolution_clock::now();
        double duration = std::chrono::duration<double>(end-start).count();

        //printf("FPS: %.0f\n", 1.f/duration);
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
                m_camera.updateRotation(e.mousePos.x, e.mousePos.y);
            } break;

            case Event::MouseScroll:
            {
                printf("%d\n", e.scrollDelta);
            } break;
        }
    }
}

void Scene_Suzanne::update()
{
    m_camera.updateMovement();
    m_angle+=0.2f;
    
    m_rotation = Matrix::multiply(Matrix::rotate(m_angle, 0), Matrix::rotate(m_angle, 1));
    m_rotation = Matrix::multiply(m_rotation, Matrix::rotate(m_angle, 2));

    m_model = Matrix::multiply(Matrix::multiply(m_translate, m_rotation), m_scale);
    m_view = Matrix::lookAt(m_camera.position, m_camera.position + m_camera.forward, m_camera.up);
}

void Scene_Suzanne::render()
{
    m_window.clear(0x111111);

    m_window.draw(m_suzanne.vertices, &m_texture, m_model, m_view, m_projection);

    m_window.display();
}

void Scene_Suzanne::shutdown()
{

}