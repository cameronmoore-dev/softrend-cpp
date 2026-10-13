#include "AppContext.h"

AppContext::AppContext()
{
    m_window = window_create("1.Triangle  2.Cube  3.Suzanne  4.Alpha Blending  5.Shadow Mapping", 640, 480);
    m_renderer = Renderer(m_window->backbuffer);
}

AppContext::~AppContext()
{
    window_cleanup(m_window);
}