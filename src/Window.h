#pragma once

#ifndef UNICODE
#define UNICODE 1
#endif

#define JAWL_CURSOR_HIDDEN  0x1
#define JAWL_CURSOR_LOCKED  0x2
#define JAWL_CURSOR_CLAMPED 0x4

#include <stdint.h>

#include <vector>
#include <queue>
#include <string>
#include <chrono>

#include "Structs.h"
#include "Input.h"
#include "Events.h"

namespace
{
    double getTimeSeconds()
    {
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> time = now.time_since_epoch();
        return time.count();
    }
}

class Window
{
    friend class Renderer;

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        Window* wnd = nullptr;
        if (uMsg == WM_CREATE)
        {
            CREATESTRUCT* create = (CREATESTRUCT*)lParam;
            wnd = (Window*)create->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)wnd);
            wnd->m_hwnd = hwnd;
        }
        else
        {
            wnd = (Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        }

        if (wnd)
        {
            return wnd->handleMessages(uMsg, wParam, lParam);
        }
        else
        {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }
    
public:
    ~Window();
    
    bool create(LPCWSTR lpWindowName, INT width, INT height);
    void close();

    void pollMessages();
    bool pollEvents(Event &event);
    void clear(uint32_t clearColour);
    void display();

    void setWindowTitle(const std::string &title);
    void setFullscreen(bool fullscreen);
    void setFlash(UINT count, DWORD timeout, bool sound);

    bool isFocused() { return GetFocus() == m_hwnd; }

    void getPos(int &outX, int &outY);
    void getSize(int &outWidth, int & outHeight);
    void setPos(int x, int y);
    void setSize(int width, int height);

    /* TODO: set to remove */
    Vec2f& windowSize() { m_windowSize = Vec2f{ (float)m_width, (float)m_height }; return m_windowSize; }
    /* -- */

    void cursorEnableFlags(uint8_t flags);
    void cursorDisableFlags(uint8_t flags);

    bool isOpen() { return m_isOpen; }

    /* TODO: move to renderer */
    std::vector<uint32_t>   m_backBuffer;
    std::vector<float>      m_depthBuffer;
    /* -- */
    

private:
    HWND        m_hwnd;
    BITMAPINFO  m_frameInfo;
    HBITMAP     m_bitmapHandle = 0;
    HDC         m_deviceContext = 0;
    RECT        m_rect;

    /* TODO: move to renderer */
    uint32_t*   m_frontBuffer = nullptr;

    Input m_input;
    std::queue<Event> m_events;
    
    /* TODO: set to remove */
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    Vec2f m_windowSize = { 0.f, 0.f };
    /* -- */

    bool m_isOpen = false;
    uint8_t m_cursorState = 0;

private:
    LPCWSTR className() const { return L"WindowClassName"; }
    LRESULT handleMessages(UINT uMsg, WPARAM wParam, LPARAM lParam);
    
    void setup();
    void registerInputDevices();
    void updateCursor();

    void onWindowResize(LPARAM lParam);
};