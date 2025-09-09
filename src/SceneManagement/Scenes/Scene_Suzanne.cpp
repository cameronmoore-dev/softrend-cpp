#include "Scene_Suzanne.h"

#include <chrono>

Scene_Suzanne::Scene_Suzanne(Window& window) :
    m_window(window)
{
}

void Scene_Suzanne::start()
{
    shader.image = &m_texture;

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
                m_camera.updateRotation(e.mousePos.x, e.mousePos.y, m_dt);
            } break;
        }
    }
}

void Scene_Suzanne::update()
{
    m_camera.updateMovement(m_dt);
    m_angle+=20.f * m_dt;
    
    m_rotation = Matrix::multiply(Matrix::rotate(m_angle, 0), Matrix::rotate(m_angle, 1));
    m_rotation = Matrix::multiply(m_rotation, Matrix::rotate(m_angle, 2));

    m_model = Matrix::multiply(Matrix::multiply(m_translate, m_rotation), m_scale);
    m_camera.updateView();

    shader.model = m_model;
    shader.view = m_camera.view();
    shader.projection = m_camera.projection();
}

void Scene_Suzanne::render()
{
    m_window.clear(0x111111);

    Vec3f centre{ 0, 0, 0 };
    float radius = 1.f;
    if (m_camera.isSphereInsideFrustum(shader.model, centre, radius))
    {
        m_renderer.draw(m_suzanne.vertices, shader);
    }

    m_window.display();
}

void Scene_Suzanne::shutdown()
{

}