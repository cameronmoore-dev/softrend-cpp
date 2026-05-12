#include "Scene_MainMenu.h"

Scene_MainMenu::Scene_MainMenu(jawl::Window &window) :
    m_window(window)
{
    m_renderer = echo::Renderer(m_window.renderTarget());
}

void Scene_MainMenu::start()
{
    m_window.setTitle("1: Tri, 2: Cube, 3: Suzanne, 4: AlphaBlending, 5: ShadowMapping");
    while (m_window.isOpen())
    {
        processInput();
        update();
        render();
    }
}

void Scene_MainMenu::processInput()
{
    jawl::Event e;
    while (m_window.pollEvents(e))
    {
        switch (e.type)
        {
            case jawl::Event::KeyPressed:
            {
                if (e.keyCode == jawl::getKey(jawl::Key::Escape))
                {
                    m_window.close();
                }

                else if (e.keyCode == jawl::getKey(jawl::Key::Num1))
                {
                    // scene_manager::addScene<Scene_Triangle>(m_window);
                }
                else if (e.keyCode == jawl::getKey(jawl::Key::Num2))
                {
                    scene_manager::addScene<Scene_Cube>(m_window);
                }
                else if (e.keyCode == jawl::getKey(jawl::Key::Num3))
                {
                    scene_manager::addScene<Scene_Suzanne>(m_window);
                }
                else if (e.keyCode == jawl::getKey(jawl::Key::Num4))
                {
                    scene_manager::addScene<Scene_AlphaBlending>(m_window);
                }
                else if (e.keyCode == jawl::getKey(jawl::Key::Num5))
                {
                    scene_manager::addScene<Scene_Assimp>(m_window);
                }
            } break;
        }
    }
}

void Scene_MainMenu::update()
{

}

void Scene_MainMenu::render()
{
    m_renderer.clear(0x412E59);
    m_renderer.endDraw();

    m_window.swapBuffers();
}

void Scene_MainMenu::shutdown()
{

}