#include "Window.h"

#include <windowsx.h>

#include <cstring>
#include <cmath>

#include <iostream>

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
        case WM_DEVICECHANGE:
        {
            if (wParam == 0x0007)
            {
                m_input.storeGameControllers();
            }
        } break;

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
                m_input.pollGamepads(raw->data.hid, m_events);
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

    m_input.storeGameControllers();
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

    RegisterRawInputDevices(rid, 3, sizeof(RAWINPUTDEVICE));
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