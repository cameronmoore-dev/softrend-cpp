#pragma once

#ifndef UNICODE
#define UNICODE 1
#endif

#define JAWL_CURSOR_HIDDEN  0x1
#define JAWL_CURSOR_LOCKED  0x2
#define JAWL_CURSOR_CLAMPED 0x4

#if(_WIN32)
    #define JAWL_STYLE_TITLEBAR     WS_CAPTION
    #define JAWL_STYLE_RESIZABLE    WS_SIZEBOX
    #define JAWL_STYLE_MAXIMISED    WS_MAXIMIZE
    #define JAWL_STYLE_MAXIMISABLE  WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX
    #define JAWL_STYLE_MINIMISABLE  WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX
    #define JAWL_STYLE_DEFAULT      WS_OVERLAPPEDWINDOW
#endif

#if(_WIN32)
    #define JAWL_CONTEXT_CORE_PROFILE       0x1
    #define JAWL_CONTEXT_COMPAT_PROFILE     0x2
#endif

#include <stdint.h>

#include <array>
#include <vector>
#include <queue>
#include <string>
#include <chrono>

#include "Structs.h"
#include "Input.h"
#include "Events.h"

typedef struct EchoBuffersInfo EchoBuffersInfo_t;

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
    Window();

    enum RendererType
    {
        Software = 0,
        Hardware
    };

    enum Hints
    {
        Renderer = 0,
        ColourBits,
        DepthBits,
        StencilBits,
        Samples,
        sRGBCapable,
        DoubleBuffer,
        ContextVerMajor,
        ContextVerMinor,
        ContextProfile,
        HintsCount
    };

    void setHint(Hints index, uint8_t value);
    bool create(const std::string &title, uint16_t width, uint16_t height, uint32_t style);
    void close();
    void cleanup();

    void pollMessages();
    bool pollEvents(Event &event);
    void clear(uint32_t clearColour);
    void swapBuffers();
    void swapInterval(uint8_t interval);

    void setWindowTitle(const std::string &title);
    void setFullscreen(bool fullscreen);
    void setFlash(UINT count, DWORD timeout, bool sound);
    
    void getPos(int &outX, int &outY);
    void getSize(int &outWidth, int & outHeight);
    void setPos(int x, int y);
    void setSize(int width, int height);
    
    void cursorEnableFlags(uint8_t flags);
    void cursorDisableFlags(uint8_t flags);
    
    bool isFocused()    { return GetFocus() == m_hwnd; }
    bool isOpen()       { return m_isOpen; }

    EchoBuffersInfo_t *getEBI();

private:
    HWND        m_hwnd;
    BITMAPINFO  m_frameInfo;
    HBITMAP     m_bitmapHandle;
    HDC         m_deviceContext;
    RECT        m_rect;

    uint32_t* m_frontBuffer = nullptr;
    EchoBuffersInfo_t *m_ebi = nullptr;
    std::vector<uint32_t> m_backBuffer;
    std::vector<float> m_depthBuffer;

    Input m_input;
    std::queue<Event> m_events;
    std::array<Hints, HintsCount> m_hints;
    
    uint16_t m_width = 0;
    uint16_t m_height = 0;
    uint8_t m_cursorState = 0;
    bool m_isOpen = false;

private:
    LPCWSTR className() const { return L"WindowClassName"; }
    LRESULT handleMessages(UINT uMsg, WPARAM wParam, LPARAM lParam);

    HWND win32CreateWindow(const LPCWSTR lpWindowName, INT width, INT height, DWORD style);

    void swSetup();
    void glSetup(const LPCWSTR title, const DWORD style);
    void registerInputDevices();
    void updateCursor();
    
    void onWindowResize(LPARAM lParam);
};