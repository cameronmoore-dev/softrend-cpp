#include "base_window.h"

namespace jawl
{
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