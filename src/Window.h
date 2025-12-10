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

#include "Events.h"
#include "Structs.h"

/* https://www.psdevwiki.com/ps4/DS4-USB#Data_Format */

/* Index into the byte array, 0 is the report ID */
#define DS4_LEFT_STICK_X    1
#define DS4_LEFT_STICK_Y    2
#define DS4_RIGHT_STICK_X   3
#define DS4_RIGHT_STICK_Y   4
#define DS4_FACE_BUTTONS    5
#define DS4_OTHER           6

/* All these button are packed into a single byte, with the dpad taking 4 bits, and the rest of the face buttons taking the remaining 4 bits */
#define DS4_DPAD_UP         0x0
#define DS4_DPAD_RIGHT      0x2
#define DS4_DPAD_DOWN       0x4
#define DS4_DPAD_LEFT       0x6
#define DS4_FB_SQUARE       4
#define DS4_FB_CROSS        5
#define DS4_FB_CIRCLE       6
#define DS4_FB_TRIANGLE     7

#define DS4_OTHER_L1        0
#define DS4_OTHER_R1        1
#define DS4_OTHER_L2        2
#define DS4_OTHER_R2        3
#define DS4_OTHER_SHARE     4
#define DS4_OTHER_OPTIONS   5


/* https://github.com/nefarius/ViGEmBus/issues/40 */
/* The second 2 bytes used for left Z and right Z were removed since this report descriptor is close to the controller I use but not exact */
/* This descriptor has a total of 18 bytes of info, where as my controller produces only 16 bytes */

/* 2 bytes used for each joystick */
#define XBOX_LEFT_STICK_X_LO 1
#define XBOX_LEFT_STICK_X_HI 2
#define XBOX_LEFT_STICK_Y_LO 3
#define XBOX_LEFT_STICK_Y_HI 4
#define XBOX_RIGHT_STICK_X_LO 5
#define XBOX_RIGHT_STICK_X_HI 6
#define XBOX_RIGHT_STICK_Y_LO 7
#define XBOX_RIGHT_STICK_Y_HI 8

#define XBOX_LEFT_Z 9
#define XBOX_RIGHT_Z 10

/* A full byte is used for the first eight buttons, then 2 bits of another byte are used for the last two */
#define XBOX_BUTTONS_LO 11
#define XBOX_BUTTONS_HI 12
#define XBOX_BUTTON_A 0x1
#define XBOX_BUTTON_B 0x2
#define XBOX_BUTTON_X 0x4
#define XBOX_BUTTON_Y 0x8
#define XBOX_BUTTON_LB 0x10
#define XBOX_BUTTON_RB 0x20
#define XBOX_BUTTON_VIEW 0x40
#define XBOX_BUTTON_MENU 0x80
#define XBOX_BUTTON_LSB 0x1
#define XBOX_BUTTON_RSB 0x2

#define XBOX_DPAD   13
#define XBOX_DPAD_UP 0x1
#define XBOX_DPAD_RIGHT 0x3
#define XBOX_DPAD_DOWN 0x5
#define XBOX_DPAD_LEFT 0x7

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
    bool pollEvents(Event& event);
    void clear(uint32_t clearColour);
    void display();

    void setWindowTitle(const std::string& title);
    Vec2f& windowSize() { m_windowSize = Vec2f{ (float)m_width, (float)m_height }; return m_windowSize; }

    void hideLockCursor(bool state);

    bool isOpen() { return m_isOpen; }

    //
    std::vector<uint32_t>   m_backBuffer;
    std::vector<float>      m_depthBuffer;
    //

private:
    HWND        m_hwnd;
    BITMAPINFO  m_frameInfo;
    HBITMAP     m_bitmapHandle = 0;
    HDC         m_deviceContext = 0;
    RECT        m_rect;

    //
    uint32_t*   m_frontBuffer = nullptr;
    //

    std::queue<Event> m_events;
    
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    Vec2f m_windowSize = { 0.f, 0.f };

    bool m_isOpen = false;
    bool m_mouseLocked = false;

private:
    LPCWSTR className() const { return L"WindowClassName"; }
    LRESULT handleMessages(UINT uMsg, WPARAM wParam, LPARAM lParam);
    
    void setup();
    void registerInputDevices();

    void onWindowResize(LPARAM lParam);
};