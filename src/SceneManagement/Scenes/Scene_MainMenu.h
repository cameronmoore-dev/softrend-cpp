#pragma once

#include "jawl/jawl.hpp"
#include "echo-renderer/renderer.h"

#include "../Scene.h"
#include "../SceneManager.h"
#include "Scene_Triangle.h"
#include "Scene_Cube.h"
#include "Scene_Suzanne.h"
#include "Scene_AlphaBlending.h"
#include "Scene_OpenGL.h"
#include "Scene_Assimp.h"

class Scene_MainMenu : public Scene
{
public:
    Scene_MainMenu(jawl::Window &window);

    void start() override;
    
protected:
    void processInput() override;
    void update() override;
    void render() override;
    void shutdown() override;

private:
    jawl::Window &m_window;
    echo::Renderer m_renderer;

    SceneManager m_sceneManager;
};