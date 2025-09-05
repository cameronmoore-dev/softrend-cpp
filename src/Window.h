#pragma once

#ifndef UNICODE
#define UNICODE 1
#endif

#include <windows.h>
#include <stdint.h>
#include <vector>
#include <queue>
#include <string>

#include <chrono>

#include "Renderer.h"
#include "Events.h"
#include "Structs.h"

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
    bool pollEvents(Event& event);
    void clear(uint32_t clearColour);
    void draw(std::vector<Vertex>& vertices, Image* texture, const Mat4& model, const Mat4& view, const Mat4& projection);
    void display();

    void setWindowTitle(const std::string& title);
    const Vec2f windowSize() const { return { (float)m_width, (float)m_height }; }

    bool isOpen() { return m_isOpen; }

private:
    HWND        m_hwnd;
    BITMAPINFO  m_frameInfo;
    HBITMAP     m_bitmapHandle = 0;
    HDC         m_deviceContext = 0;
    RECT        m_rect;

    uint32_t*               m_frontBuffer = nullptr;
    std::vector<uint32_t>   m_backBuffer;
    std::vector<float>      m_depthBuffer;

    Renderer                m_renderer;
    std::queue<Event>       m_events;

    int xorigin = 0;
    int yorigin = 0;
    uint32_t m_width = 0;
    uint32_t m_height = 0;

    bool m_isOpen = false;

private:
    LPCWSTR className() const { return L"WindowClassName"; }
    LRESULT handleMessages(UINT uMsg, WPARAM wParam, LPARAM lParam);
    
    void setup();
    void registerInputDevices();

    void onWindowResize(LPARAM lParam);
};