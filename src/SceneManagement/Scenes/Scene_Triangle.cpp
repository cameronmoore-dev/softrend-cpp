#include "Scene_Triangle.h"

#include "vertex.h"
#include "SceneManagement/SceneManager.h"

#include <cstdio>

Scene_Triangle::Scene_Triangle(AppContext &app) :
    m_app(app)
{
}

void Scene_Triangle::start()
{
    m_app.getRenderer().setVertexAttrib(sizeof(Vertex), 0, 4); /* Position */
    m_app.getRenderer().setVertexAttrib(sizeof(Vertex), 1, 3); /* Normal */
    m_app.getRenderer().setVertexAttrib(sizeof(Vertex), 2, 2); /* TexCoord */
    m_app.getRenderer().setVertexAttrib(sizeof(Vertex), 3, 3); /* Tangent */
    m_app.getRenderer().setVertexAttrib(sizeof(Vertex), 4, 3); /* Bitangent */
    m_app.getRenderer().setVertexAttrib(sizeof(Vertex), 5, 3); /* Colour */

    m_triangle.resize(3);
    m_triangle[0].position = { -0.5f, -0.5, 0 };
    m_triangle[0].colour = hexToRGB(0xFF0000);
    m_triangle[1].position = { 0.5f, -0.5f, 0 };
    m_triangle[1].colour = hexToRGB(0x0000FF);
    m_triangle[2].position = { 0, 0.5f, 0 };
    m_triangle[2].colour = hexToRGB(0x00FF00);

    m_app.getRenderer().bindShader(m_shader);

    timepoint_ previous = get_time();
    while (window_is_open(&m_app.getWindow()))
    {
        timepoint_ current = get_time();
        duration_ diff = timepoint_diff(&current, &previous);
        f32 dt = (f32)diff.us / 1000000L;
        previous = current;

        processInput();
        update();
        render();

        window_sleep(&m_app.getWindow());

        printf("FPS: %.1f\n", 1.f/dt);
    }
}

void Scene_Triangle::processInput()
{
    window_pump_messages(&m_app.getWindow());

    if (is_key_pressed(&m_app.getWindow().input, ESC))
    {
        shutdown();
    }
}

void Scene_Triangle::update()
{
}

void Scene_Triangle::render()
{
    m_app.getRenderer().clear(0x111111);
    m_app.getRenderer().drawArray(m_triangle.data(), m_triangle.size());
    m_app.getRenderer().endDraw();

    window_swap_buffers(&m_app.getWindow());
}

void Scene_Triangle::shutdown()
{
    scene_manager::removeScene();
}