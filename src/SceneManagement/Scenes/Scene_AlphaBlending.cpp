#include "Scene_AlphaBlending.h"

#include "../../Events.h"

Scene_AlphaBlending::Scene_AlphaBlending(Window& window) : 
    m_window(window)
{
}

void Scene_AlphaBlending::start()
{
    m_grassShader.texture = &m_grassTexture;
    m_cubeShader.texture = &m_dirtTexture;
    m_windowShader.texture = &m_windowTexture;

    m_rotation = Matrix::rotate(-180, 1);

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

void Scene_AlphaBlending::processInput()
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

void Scene_AlphaBlending::update()
{
    m_camera.updateMovement(m_dt);
    m_camera.updateView();

    m_angle+=20.f * m_dt;
    m_cubeRotation = Matrix::multiply(Matrix::rotate(m_angle, 0), Matrix::rotate(m_angle, 1));
    m_cubeRotation = Matrix::multiply(m_cubeRotation, Matrix::rotate(m_angle, 2));
    
    m_grassShader.view = m_camera.view();
    m_grassShader.projection = m_camera.projection();

    m_cubeShader.view = &m_camera.view();
    m_cubeShader.projection = &m_camera.projection();

    m_windowShader.view = &m_camera.view();
    m_windowShader.projection = &m_camera.projection();
}

void Scene_AlphaBlending::render()
{
    m_window.clear(0x111111);

    m_cubeTranslate = Matrix::translate({ -5.f, 0.f, 5.f });
    m_cubeModel = Matrix::multiply(Matrix::multiply(m_cubeTranslate, m_cubeRotation), m_scale);
    m_cubeShader.model = &m_cubeModel;
    m_renderer.draw(m_cube.vertices, m_cubeShader);

    m_grassModel = Matrix::multiply(Matrix::multiply(m_grassTranslate, m_rotation), m_scale);
    m_grassShader.model = m_grassModel;
    m_renderer.draw(m_plane.vertices, m_grassShader);

    m_windowTranslate = Matrix::translate({ 5.f, 0.f, -3.f });
    m_windowModel = Matrix::multiply(Matrix::multiply(m_windowTranslate, m_rotation), m_scale);
    m_windowShader.model = &m_windowModel;
    m_renderer.draw(m_plane.vertices, m_windowShader);

    m_window.display();
}

void Scene_AlphaBlending::shutdown()
{
    
}