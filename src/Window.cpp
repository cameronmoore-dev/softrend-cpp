#include "Window.h"

#include <windowsx.h>

#include <cstring>
#include <cmath>

#include <iostream>

#include <glad/glad.h>
#include <GL/wglext.h>

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

    ClipCursor(nullptr);
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

bool Window::create(LPCWSTR lpWindowName, INT width, INT height, bool isGL)
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
        m_isGL = isGL;
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
    m_deviceContext = CreateCompatibleDC(0);
    if (!m_isGL)
    {
        m_frameInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        m_frameInfo.bmiHeader.biPlanes = 1;
        m_frameInfo.bmiHeader.biBitCount = 32;
        m_frameInfo.bmiHeader.biCompression = BI_RGB;
    }
    else 
    {
        glSetup();
    }

    GetWindowRect(m_hwnd, &m_rect);
    ShowWindow(m_hwnd, 1);
    SetFocus(m_hwnd);

    m_input.storeGameControllers();
}

void Window::glSetup()
{
    m_deviceContext = GetDC(m_hwnd);

    PIXELFORMATDESCRIPTOR pfd;
    ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 4;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int old_format = ChoosePixelFormat(m_deviceContext, &pfd);
    SetPixelFormat(m_deviceContext, old_format, &pfd);

    HGLRC hrc;
    hrc = wglCreateContext(m_deviceContext);
    wglMakeCurrent(m_deviceContext, hrc);

    gladLoadGL();

    PFNWGLSWAPINTERVALEXTPROC wglSwapInterval = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");

    wglDeleteContext(hrc);

    int formats = 0;
    UINT numFormats = 0;
    int formatAttribs[] =
    {
        WGL_DRAW_TO_WINDOW_ARB,     GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB,     GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB,      GL_TRUE,
        WGL_ACCELERATION_ARB,       WGL_FULL_ACCELERATION_ARB,
        WGL_PIXEL_TYPE_ARB,         WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB,         32,
        WGL_DEPTH_BITS_ARB,         24,
        WGL_STENCIL_BITS_ARB,       8,
        0
    };

    int format = wglChoosePixelFormatARB(m_deviceContext, formatAttribs, 0, 1, &formats, &numFormats);
    int attribList[] =
    { 
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 1,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };
    HGLRC hRenderContext = wglCreateContextAttribsARB(m_deviceContext, 0, attribList);

    wglMakeCurrent(m_deviceContext, hRenderContext);

    wglSwapInterval(1);
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

bool Window::pollEvents(Event &event)
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
    updateCursor();
    
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

void Window::swapBuffers()
{
    SwapBuffers(m_deviceContext);
}

void Window::setFlash(UINT count, DWORD timeout, bool sound)
{
    FLASHWINFO info;
    info.cbSize = sizeof(FLASHWINFO);
    info.hwnd = m_hwnd;
    info.dwFlags = FLASHW_ALL;
    info.uCount = count;
    info.dwTimeout = timeout;

    ::FlashWindowEx(&info);
    if (sound)
    {
        ::MessageBeep(MB_OK);
    }
}

void Window::setWindowTitle(const std::string &title)
{
    LPCWSTR str = std::wstring(title.begin(), title.end()).c_str();
    SetWindowTextW(m_hwnd, str);
}

void Window::getPos(int &outX, int &outY)
{
    ::GetWindowRect(m_hwnd, &m_rect);
    outX = m_rect.left;
    outY = m_rect.top;
}

void Window::getSize(int &outWidth, int & outHeight)
{
    ::GetWindowRect(m_hwnd, &m_rect);
    outWidth = m_rect.right - m_rect.left;
    outHeight = m_rect.bottom - m_rect.top;
}

void Window::setPos(int x, int y)
{
    ::SetWindowPos(m_hwnd, NULL, x, y, m_rect.right - m_rect.left, m_rect.bottom - m_rect.top, SWP_FRAMECHANGED);
}

void Window::setSize(int width, int height)
{
    ::SetWindowPos(m_hwnd, NULL, m_rect.left, m_rect.top, m_rect.left + width, m_rect.top + height, SWP_FRAMECHANGED);
}

void Window::setFullscreen(bool fullscreen)
{
    if (!fullscreen)
    {
        HMONITOR hMonitor = MonitorFromWindow(m_hwnd, MONITOR_DEFAULTTONEAREST);
        MONITORINFOEX info;
        info.cbSize = sizeof(MONITORINFOEX);
        DWORD fsStyle = WS_POPUP | WS_VISIBLE;

        GetMonitorInfo(hMonitor, &info);
        SetWindowLongPtr(m_hwnd, GWL_STYLE, fsStyle);
        SetWindowPos(m_hwnd, NULL, info.rcMonitor.left, info.rcMonitor.top, info.rcMonitor.right, info.rcMonitor.bottom, SWP_FRAMECHANGED);
        return;
    }

    /* TODO: add a way to keep a non-fullscreen window config to default back to */
    SetWindowLongPtr(m_hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
    SetWindowPos(m_hwnd, NULL, m_rect.left, m_rect.top, m_rect.right, m_rect.bottom, SWP_FRAMECHANGED);
}

void Window::updateCursor()
{
    if (m_cursorState & JAWL_CURSOR_LOCKED)
    {
        SetCursorPos(m_rect.left + (m_width / 2), m_rect.bottom - (m_height / 2));
    }

    if (m_cursorState & JAWL_CURSOR_CLAMPED)
    {
        ClipCursor(&m_rect);
    }

    ShowCursor(!(m_cursorState & JAWL_CURSOR_HIDDEN));
}

void Window::cursorEnableFlags(uint8_t state)
{
    m_cursorState |= state;
}

void Window::cursorDisableFlags(uint8_t state)
{
    m_cursorState &= ~(state);
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