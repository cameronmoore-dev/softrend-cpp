#include "Scene_OpenGL.h"

Scene_OpenGL::Scene_OpenGL(Window &window, Renderer&) : 
    m_window(window)
{
    m_shader = GLShader("resources/shaders/default.vert", "resources/shaders/default.frag");

    m_vertices.resize(3);
    m_vertices[0].pos = { 0, 0.5f };
    m_vertices[0].colour = { 0, 0, 1 };

    m_vertices[1].pos = { 0.5f, -0.5f };
    m_vertices[1].colour = { 0, 1, 0 };

    m_vertices[2].pos = { -0.5, -0.5f };
    m_vertices[2].colour = { 1, 0, 0 };
}

void Scene_OpenGL::start()
{
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(VERTEX), &m_vertices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VERTEX), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VERTEX), (void*)offsetof(VERTEX, colour));

    glBindVertexArray(0);

    double previous = getTimeSeconds();
    while (m_window.isOpen())
    {
        double current = getTimeSeconds();
        m_dt = current-previous;
        previous = current;

        processInput();
        update();
        render();

        // printf("FPS: %.0f\n", 1.f/m_dt);
    }

    glDeleteVertexArrays(1, &m_vao);
	glDeleteBuffers(1, &m_vbo);
}

void Scene_OpenGL::processInput()
{
    Event e;
    while (m_window.pollEvents(e))
    {
        switch (e.type)
        {
            case Event::KeyPressed:
            {
                if (e.keyCode == Input::getKey(Input::JAWL_KEY::Escape))
                {
                    m_window.close();
                }
            } break;

            default: break;
        }
    }
}

void Scene_OpenGL::update()
{
    m_vertices[0].pos.x += 0.1f * m_dt;
    m_vertices[1].pos.x -= 0.1f * m_dt;
    m_vertices[2].pos.y += 0.1f * m_dt;

    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(VERTEX), &m_vertices[0], GL_DYNAMIC_DRAW);
}

void Scene_OpenGL::render()
{
    glClearColor(0.1, 0.1, 0.1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    m_shader.Use();
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    m_window.swapBuffers();
}

void Scene_OpenGL::shutdown()
{
    
}