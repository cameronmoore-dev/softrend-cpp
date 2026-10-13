#include "AppContext.h"

#include "SceneManagement/SceneManager.h"
#include "SceneManagement/Scenes/Scene_MainMenu.h"

int main()
{
    srand(static_cast<time_t>(time(0)));

    AppContext app;

    scene_manager::addScene<Scene_MainMenu>(app);

    return 0;
}