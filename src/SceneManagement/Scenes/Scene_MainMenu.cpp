#include "Scene_MainMenu.h"

#include "../SceneManager.h"
#include "Scene_Triangle.h"
#include "Scene_Cube.h"
#include "Scene_Suzanne.h"
#include "Scene_AlphaBlending.h"
#include "Scene_Assimp.h"

Scene_MainMenu::Scene_MainMenu(AppContext &app) :
    m_app(app)
{
}

void Scene_MainMenu::start()
{
    m_app.getRenderer().cleanup();

    while (window_is_open(&m_app.getWindow()))
    {
        processInput();
        update();
        render();

        window_sleep(&m_app.getWindow());
    }
}

void Scene_MainMenu::processInput()
{
    window_pump_messages(&m_app.getWindow());

    if (is_key_pressed(&m_app.getWindow().input, ESC))
    {
        shutdown();
    }

    else if (is_key_pressed(&m_app.getWindow().input, NUM1))
    {
        scene_manager::addScene<Scene_Triangle>(m_app);
    }

    else if (is_key_pressed(&m_app.getWindow().input, NUM2))
    {
        scene_manager::addScene<Scene_Cube>(m_app);
    }

    else if (is_key_pressed(&m_app.getWindow().input, NUM3))
    {
        scene_manager::addScene<Scene_Suzanne>(m_app);
    }

    else if (is_key_pressed(&m_app.getWindow().input, NUM4))
    {
        scene_manager::addScene<Scene_AlphaBlending>(m_app);
    }

    else if (is_key_pressed(&m_app.getWindow().input, NUM5))
    {
        scene_manager::addScene<Scene_Assimp>(m_app);
    }
}

void Scene_MainMenu::update()
{
}

void Scene_MainMenu::render()
{
    m_app.getRenderer().clear(0x412E59);
    m_app.getRenderer().endDraw();

    window_swap_buffers(&m_app.getWindow());
}

void Scene_MainMenu::shutdown()
{
    window_close(&m_app.getWindow());
}