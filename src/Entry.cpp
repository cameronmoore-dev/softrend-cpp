#include <iostream>

#include "SceneManagement/SceneManager.h"
#include "SceneManagement/Scenes/Scene_Cube.h"
#include "SceneManagement/Scenes/Scene_Triangle.h"
#include "SceneManagement/Scenes/Scene_Suzanne.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    srand(static_cast<unsigned int>(time(0)));

    Window win;
    if (!win.create("Software Renderer", WS_OVERLAPPEDWINDOW))
    {
        std::cerr << "ERROR::WINDOW::failed to create!" << std::endl;
        return 1;
    }

    SceneManager sceneManager;
    sceneManager.addScene<Scene_Suzanne>(window);

    return 0;
}