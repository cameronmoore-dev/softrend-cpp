#include <glad/gl.h>

#include "jawl/jawl.hpp"

#include "SceneManagement/SceneManager.h"
#include "SceneManagement/Scenes/Scene_Cube.h"
#include "SceneManagement/Scenes/Scene_Triangle.h"
#include "SceneManagement/Scenes/Scene_Suzanne.h"
#include "SceneManagement/Scenes/Scene_AlphaBlending.h"
#include "SceneManagement/Scenes/Scene_OpenGL.h"
#include "SceneManagement/Scenes/Scene_Assimp.h"

int main()
{
    srand(static_cast<UINT>(time(0)));

    jawl::Window *window = jawl::init();
    if (!window)
    {
        return -1;
    }

    window->setHint(jawl::ContextVerMajor, 4);
    window->setHint(jawl::ContextVerMinor, 1);
    window->setHint(jawl::ContextProfile, jawl::GLContextProfile::Core);
    window->setHint(jawl::Samples, 4);

    window->create("JAWL Window", 640, 480, jawl::Style::Default, jawl::RendererType::Software);

    // if (!gladLoaderLoadGL())
    // {
    //     printf("Failed to load GLAD!\n");
    //     return EXIT_FAILURE;
    // }

    window->swapInterval(1);
    window->cursorEnableFlags(jawl::CursorFlags::Locked | jawl::CursorFlags::Hidden | jawl::CursorFlags::Clamped);

    SceneManager sceneManager;
    sceneManager.addScene<Scene_Cube>(*window);

    window->cleanup();
    delete(window);

    return 0;
}