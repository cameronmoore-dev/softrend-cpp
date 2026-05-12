#include "jawl/jawl.hpp"

#include "SceneManagement/SceneManager.h"
#include "SceneManagement/Scenes/Scene_MainMenu.h"

int main()
{
    srand(static_cast<time_t>(time(0)));

    jawl::Window *window = jawl::init();
    if (!window)
    {
        return -1;
    }

    window->create("JAWL Window", 640, 480, jawl::Style::Default);

    window->cursorEnableFlags(jawl::CursorFlags::Locked | jawl::CursorFlags::Hidden | jawl::CursorFlags::Clamped);

    scene_manager::addScene<Scene_MainMenu>(*window);

    window->cleanup();
    delete(window);

    return 0;
}