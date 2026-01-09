#include "base_window.h"

#include <cstring>

namespace jawl
{
    Window::Window()
    {
        /* Set default values to window hints */
        setHint(ColourBits, 32);
        setHint(DepthBits, 24);
        setHint(StencilBits, 8);
        setHint(Samples, 0);
        setHint(sRGBCapable, 0);
        setHint(DoubleBuffer, 1);
        setHint(ContextVerMajor, 3);
        setHint(ContextVerMinor, 3);
        setHint(ContextProfile, jawl::GLContextProfile::Core);
    }
    
    void Window::setHint(Hints index, uint8_t value)
    {
        m_hints[index] = (Hints)value;
    }
    
    void Window::close()
    {
        m_isOpen = false;
    }
    
    bool Window::pollEvents(Event &event)
    {
        if (m_events.empty())
        {
            return false;
        }
    
        event = m_events.front();
        m_events.pop();
    
        return true;    
    }

    void Window::cursorEnableFlags(const uint8_t state)
    {
        m_cursorState |= state;
    }
    
    void Window::cursorDisableFlags(const uint8_t state)
    {
        m_cursorState &= ~(state);
    }
}