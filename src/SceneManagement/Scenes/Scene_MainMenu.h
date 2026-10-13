#pragma once

#include "AppContext.h"

#include "../Scene.h"

class Scene_MainMenu : public Scene
{
public:
    Scene_MainMenu(AppContext &app);

    void start() override;
    
protected:
    void processInput() override;
    void update() override;
    void render() override;
    void shutdown() override;

private:
    AppContext &m_app;
};