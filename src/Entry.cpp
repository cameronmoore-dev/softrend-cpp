#include <windows.h>
#include <iostream>

#include "SceneManagement/SceneManager.h"
#include "SceneManagement/Scenes/Scene_Cube.h"
#include "SceneManagement/Scenes/Scene_Triangle.h"
#include "SceneManagement/Scenes/Scene_Suzanne.h"
#include "SceneManagement/Scenes/Scene_AlphaBlending.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    srand(static_cast<UINT>(time(0)));

    Window window;
    if (!window.create(L"Software Renderer", 640, 480))
    {
        std::cerr << "ERROR::WINDOW::failed to create!" << std::endl;
        return 1;
    }

    window.cursorEnableFlags(JAWL_CURSOR_HIDDEN | JAWL_CURSOR_HIDDEN | JAWL_CURSOR_CLAMPED);

    SceneManager sceneManager;
    sceneManager.addScene<Scene_Suzanne>(window);

    window.~Window();

    return 0;
}