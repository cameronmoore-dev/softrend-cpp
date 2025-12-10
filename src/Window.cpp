#include "Window.h"

#include <windowsx.h>
#include <cstring>
#include <cmath>

#include <iostream>


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

// BYTE zl = raw->data.hid.bRawData[XBOX_LEFT_Z];
// BYTE zh = raw->data.hid.bRawData[XBOX_RIGHT_Z];
// std::cout << "Z: " << ((uint16_t)zl & 0x03 | (((uint16_t)zh) & 0xFF) << 2) << std::endl;


// BYTE dpad = raw->data.hid.bRawData[XBOX_DPAD];
// if (((uint16_t)dpad) == XBOX_DPAD_UP)
// {
//     std::cout << "dpad up" << std::endl;
// }

// else if (((uint16_t)dpad) == XBOX_DPAD_RIGHT)
// {
//     std::cout << "dpad right" << std::endl;
// }

// else if (((uint16_t)dpad) == XBOX_DPAD_DOWN)
// {
//     std::cout << "dpad down" << std::endl;
// }

// else if (((uint16_t)dpad) == XBOX_DPAD_LEFT)
// {
//     std::cout << "dpad left" << std::endl;
// }

Window::~Window()
{
    if (m_bitmapHandle)
    {
        DeleteObject(m_bitmapHandle);
    }
    
    if (m_deviceContext)
    {
        DeleteDC(m_deviceContext);
    }
}

LRESULT Window::handleMessages(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_CLOSE:
        {
            close();
        } break;

        case WM_SIZE:
        {
            onWindowResize(lParam);
        } break;

        case WM_INPUT:
        {
            UINT size = 0;
            GetRawInputData((HRAWINPUT)lParam, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER));

            BYTE buffer[size] = {};
            if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, buffer, &size, sizeof(RAWINPUTHEADER)) != size)
            {
                break;
            }

            RAWINPUT* raw = (RAWINPUT*)buffer;
            if (raw->header.dwType == RIM_TYPEHID)
            {
                Event e;
                e.type = Event::Gamepad;
                for (int i = 0; i < 13; i++)
                {
                    e.gamepadData[i] = raw->data.hid.bRawData[i];
                }

                m_events.push(e);
            }

            if (raw->header.dwType == RIM_TYPEKEYBOARD)
            {
                Event e;
                e.keyCode = raw->data.keyboard.VKey;
                
                if (raw->data.keyboard.Message == WM_KEYDOWN)
                {
                    e.type = Event::KeyPressed;
                }
                else if (raw->data.keyboard.Message == WM_KEYUP)
                {
                    e.type = Event::KeyReleased;
                }

                m_events.push(e);
            }

            if (raw->header.dwType == RIM_TYPEMOUSE)
            {
                Event e;

                if (raw->data.mouse.usFlags == MOUSE_MOVE_RELATIVE)
                {
                    e.type = Event::MouseMove;
                    e.mousePos.x = raw->data.mouse.lLastX;
                    e.mousePos.y = raw->data.mouse.lLastY;
                }

                switch (raw->data.mouse.usButtonFlags)
                {
                    case RI_MOUSE_LEFT_BUTTON_DOWN:
                    {
                        e.type = Event::MousePressed;
                        e.mouseButton = 1;
                    } break;

                    case RI_MOUSE_RIGHT_BUTTON_DOWN:
                    {
                        e.type = Event::MousePressed;
                        e.mouseButton = 2;
                    } break;

                    case RI_MOUSE_LEFT_BUTTON_UP:
                    {
                        e.type = Event::MouseReleased;
                        e.mouseButton = 1;
                    } break;

                    case RI_MOUSE_RIGHT_BUTTON_UP:
                    {
                        e.type = Event::MouseReleased;
                        e.mouseButton = 2;
                    } break;

                    case RI_MOUSE_WHEEL:
                    {
                        e.type = Event::MouseScroll;
                        e.scrollDelta = raw->data.mouse.usButtonData / WHEEL_DELTA;
                    } break;
                }

                m_events.push(e);
            }
        } break;

        default:
            return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
    }

    return TRUE;
}

bool Window::create(LPCWSTR lpWindowName, INT width, INT height)
{
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = className();

    RegisterClass(&wc);

    m_hwnd = CreateWindowEx(
        0, 
        className(), 
        lpWindowName, 
        WS_OVERLAPPEDWINDOW, 
        CW_USEDEFAULT, CW_USEDEFAULT, 
        width, height, 
        0, 
        0, 
        GetModuleHandle(nullptr), 
        this
    );

    if (m_hwnd)
    {
        setup();
        registerInputDevices();

        m_isOpen = m_hwnd;
        return TRUE;
    }
    
    return FALSE;
}

void Window::close()
{
    m_isOpen = false;
    DestroyWindow(m_hwnd);
}

void Window::setup()
{
    m_frameInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    m_frameInfo.bmiHeader.biPlanes = 1;
    m_frameInfo.bmiHeader.biBitCount = 32;
    m_frameInfo.bmiHeader.biCompression = BI_RGB;
    m_deviceContext = CreateCompatibleDC(0);

    hideLockCursor(true);
    GetWindowRect(m_hwnd, &m_rect);
    ShowWindow(m_hwnd, 1);
    SetFocus(m_hwnd);
}

void Window::registerInputDevices()
{
    RAWINPUTDEVICE rid[3];

    /* Mouse */
    rid[0].usUsagePage = 0x01;
    rid[0].usUsage = 0x02;
    rid[0].dwFlags = 0;
    rid[0].hwndTarget = m_hwnd;

    /* Keyboard */
    rid[1].usUsagePage = 0x01;
    rid[1].usUsage = 0x06;
    rid[1].dwFlags = 0;
    rid[1].hwndTarget = m_hwnd;

    /* Generic Gamepad */
    rid[2].usUsagePage = 0x01;
    rid[2].usUsage = 0x05;
    rid[2].dwFlags = 0;
    rid[2].hwndTarget = m_hwnd;

    RegisterRawInputDevices(rid, 3, sizeof(rid[0]));
}

void Window::pollMessages()
{
    MSG msg = {};
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        DispatchMessage(&msg);
    }
}

bool Window::pollEvents(Event& event)
{
    if (m_events.empty())
    {
        pollMessages();
        return false;
    }

    event = m_events.front();
    m_events.pop();

    return true;    
}

void Window::clear(uint32_t clearColour)
{
    InvalidateRect(m_hwnd, nullptr, FALSE);
    std::fill(m_backBuffer.begin(), m_backBuffer.end(), clearColour);
    std::fill(m_depthBuffer.begin(), m_depthBuffer.end(), INFINITY);
}

void Window::display()
{
    if (m_mouseLocked)
    {
        SetCursorPos(m_rect.left + (m_width / 2), m_rect.bottom - (m_height / 2));
    }
    
    PAINTSTRUCT paint;
    HDC hdc = BeginPaint(m_hwnd, &paint);

    m_backBuffer[m_width / 2 + m_height / 2 * m_width] = 0xff00ff;

    std::memcpy(m_frontBuffer, m_backBuffer.data(), m_width*m_height*sizeof(uint32_t));
    BitBlt(hdc, 
        paint.rcPaint.left, paint.rcPaint.top, 
        m_width, m_height, 
        m_deviceContext, 
        paint.rcPaint.left, paint.rcPaint.top, 
        SRCCOPY);
    
    EndPaint(m_hwnd, &paint);
}

void Window::setWindowTitle(const std::string& title)
{
    LPCWSTR str = std::wstring(title.begin(), title.end()).c_str();
    SetWindowTextW(m_hwnd, str);
}

void Window::hideLockCursor(bool state)
{
    if (state)
    {
        
        ShowCursor(false);
        m_mouseLocked = true;
        return;
    }

    ClipCursor(nullptr);
    ShowCursor(true);
    m_mouseLocked = false;
}

void Window::onWindowResize(LPARAM lParam)
{
    m_width = LOWORD(lParam);
    m_height = HIWORD(lParam);

    m_frameInfo.bmiHeader.biWidth = m_width;
    m_frameInfo.bmiHeader.biHeight = m_height;

    if (m_bitmapHandle)
    {
        DeleteObject(m_bitmapHandle);
    }

    m_bitmapHandle = CreateDIBSection(nullptr, &m_frameInfo, DIB_RGB_COLORS, (void**)&m_frontBuffer, nullptr, 0);
    SelectObject(m_deviceContext, m_bitmapHandle);

    m_backBuffer.resize(m_width*m_height);
    m_depthBuffer.resize(m_width*m_height, INFINITY);
}