#pragma once

extern "C"
{
    #include "os/window.h"
}
#include "Renderer/renderer.h"

class AppContext
{
public:
    AppContext();
    ~AppContext();

    window_  &getWindow()   { return *m_window; }
    Renderer &getRenderer() { return m_renderer; }

private:
    window_  *m_window;
    Renderer m_renderer;
};