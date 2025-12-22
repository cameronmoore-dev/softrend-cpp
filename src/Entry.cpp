#include <windows.h>
#include <iostream>

#include "SceneManagement/SceneManager.h"
#include "SceneManagement/Scenes/Scene_Cube.h"
#include "SceneManagement/Scenes/Scene_Triangle.h"
#include "SceneManagement/Scenes/Scene_Suzanne.h"
#include "SceneManagement/Scenes/Scene_AlphaBlending.h"
#include "SceneManagement/Scenes/Scene_OpenGL.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    srand(static_cast<UINT>(time(0)));

    Window window;
    window.setHint(Window::Renderer, Window::RendererType::Software);
    window.setHint(Window::ContextVerMajor, 4);
    window.setHint(Window::ContextVerMinor, 1);
    window.setHint(Window::ContextProfile, JAWL_CONTEXT_CORE_PROFILE);
    window.setHint(Window::Samples, 4);
    int style = JAWL_STYLE_DEFAULT;

    if (!window.create("JAWL", 640, 480, style))
    {
        std::cerr << "ERROR::WINDOW::failed to create!" << std::endl;
        return 1;
    }

    Renderer renderer = Renderer(window.getEBI());

    window.swapInterval(1);
    window.cursorEnableFlags(JAWL_CURSOR_HIDDEN | JAWL_CURSOR_HIDDEN | JAWL_CURSOR_CLAMPED);

    SceneManager sceneManager;
    sceneManager.addScene<Scene_Suzanne>(window, renderer);

    return 0;
}