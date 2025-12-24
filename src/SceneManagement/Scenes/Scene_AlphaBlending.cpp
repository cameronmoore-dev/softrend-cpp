#include "Scene_AlphaBlending.h"

#include "jawl/win32/win32_window.h"

Scene_AlphaBlending::Scene_AlphaBlending(jawl::Window &window) : 
    m_window(window)
{
    int x = 0;
    int y = 0;
    m_window.getSize(x, y);
    m_camera = Camera(45.f, (float)x / (float)y, 0.5f, 20.f);

    m_renderer = Renderer(m_window.getEBI());
}

void Scene_AlphaBlending::start()
{
    m_grassShader.setUniform<Image>(m_grassShader.texture, m_grassTexture);
    m_cubeShader.setUniform<Image>(m_cubeShader.texture, m_dirtTexture);
    m_windowShader.setUniform<Image>(m_windowShader.texture, m_windowTexture);

    m_rotation = Matrix::rotate(180, 1);

    m_cubeTranslate = Matrix::translate({ -5.f, 0.f, 5.f });
    m_windowTranslate = Matrix::translate({ 5.f, 0.f, -3.f });

    m_grassModel = Matrix::multiply(Matrix::multiply(Matrix::identity(), m_rotation), m_scale);
    m_windowModel = Matrix::multiply(Matrix::multiply(m_windowTranslate, m_rotation), m_scale);

    m_grassShader.setUniform<Mat4>(m_grassShader.projection, m_camera.projection());
    m_grassShader.setUniform<Mat4>(m_grassShader.view, m_camera.view());
    m_grassShader.setUniform<Mat4>(m_grassShader.model, m_grassModel);

    m_cubeShader.setUniform<Mat4>(m_cubeShader.projection, m_camera.projection());
    m_cubeShader.setUniform<Mat4>(m_cubeShader.view, m_camera.view());
    m_cubeShader.setUniform<Mat4>(m_cubeShader.model, m_cubeModel);

    m_windowShader.setUniform<Mat4>(m_windowShader.projection, m_camera.projection());
    m_windowShader.setUniform<Mat4>(m_windowShader.view, m_camera.view());
    m_windowShader.setUniform<Mat4>(m_windowShader.model, m_windowModel);

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

void Scene_AlphaBlending::update()
{
    m_camera.updateMovement(m_dt);
    m_camera.updateView();

    m_angle+=20.f * m_dt;
    m_cubeRotation = Matrix::multiply(Matrix::rotate(m_angle, 0), Matrix::rotate(m_angle, 1));
    m_cubeRotation = Matrix::multiply(m_cubeRotation, Matrix::rotate(m_angle, 2));
    m_cubeModel = Matrix::multiply(Matrix::multiply(m_cubeTranslate, m_cubeRotation), m_scale);
}

void Scene_AlphaBlending::render()
{
    m_renderer.clear(0x111111);

    m_renderer.draw(m_cube.vertices, m_cubeShader);
    m_renderer.draw(m_plane.vertices, m_grassShader);
    m_renderer.draw(m_plane.vertices, m_windowShader);

    m_window.swapBuffers();
}

void Scene_AlphaBlending::shutdown()
{
    
}