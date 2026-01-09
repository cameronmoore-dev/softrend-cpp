#include "win32_window.h"

#include <cstring>
#include <limits>

#include <GL/gl.h>
#include <GL/wglext.h>

namespace jawl
{
    struct Backbuffer
    {
        uint16_t width;
        uint16_t height;
        uint32_t *backbuffer = nullptr;
    };

    LRESULT Win32_Window::handleMessages(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch(uMsg)
        {
            case WM_DEVICECHANGE:
            {
                if (wParam == 0x0007)
                {
                    storeGamepads();
                }
            } break;

            case WM_CLOSE:
            {
                close();
            } break;

            case WM_SIZE:
            {
                onWindowResize(lParam);

                Event e;
                e.type = Event::WindowResize;
                e.wndResizeInfo.width = LOWORD(lParam);
                e.wndResizeInfo.height = HIWORD(lParam);

                m_events.push(e);
            } break;

            case WM_INPUT:
            {
                UINT size = 0;
                ::GetRawInputData((HRAWINPUT)lParam, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER));

                BYTE buffer[size] = {};
                if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, buffer, &size, sizeof(RAWINPUTHEADER)) != size)
                {
                    break;
                }

                RAWINPUT* raw = (RAWINPUT*)buffer;
                if (raw->header.dwType == RIM_TYPEHID)
                {
                    pollGamepads(raw->data.hid, m_events);
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
                return ::DefWindowProc(m_window, uMsg, wParam, lParam);
        }

        return TRUE;
    }

    void Win32_Window::registerWinClass()
    {
        WNDCLASS wc = {};
        wc.lpfnWndProc = WndProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = className();

        ::RegisterClass(&wc);
    }

    void Win32_Window::pollMessages()
    {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
    }

    Win32_Window *Win32_Window::create(const std::string &title, const uint16_t width, const uint16_t height, const uint32_t style, const RendererType rtype)
    {
        registerWinClass();

        LPCWSTR cTitle = std::wstring(title.begin(), title.end()).c_str();
        m_window = ::CreateWindowEx(
            0, 
            className(), 
            cTitle, 
            style,
            CW_USEDEFAULT, CW_USEDEFAULT, 
            width, height, 
            0, 
            0, 
            GetModuleHandle(nullptr), 
            this
        );

        if (!m_window)
        {
            return nullptr;
        }

        m_title = title;
        m_width = width;
        m_height = height;
        m_style = style;

        m_rtype = rtype;
        m_rtype == RendererType::Software ? swSetup() : glSetup();

        ::GetWindowRect(m_window, &m_rect);
        ::ShowWindow(m_window, SW_NORMAL);
        ::SetFocus(m_window);

        registerInputDevices();
        storeGamepads();

        m_isOpen = m_window;
        return this;
    }

    void Win32_Window::swapBuffers()
    {
        updateCursor();

        if (m_rtype == RendererType::Software)
        {
            ::InvalidateRect(m_window, nullptr, FALSE);

            PAINTSTRUCT paint;
            HDC hdc = ::BeginPaint(m_window, &paint);
        
            std::memcpy(m_frontbuffer, m_backbuffer->backbuffer, m_width*m_height*sizeof(uint32_t));
            ::BitBlt(hdc, 
                     paint.rcPaint.left, paint.rcPaint.top, 
                     m_width, m_height, 
                     m_deviceContext, 
                     paint.rcPaint.left, paint.rcPaint.top, 
                     SRCCOPY);
            
            ::EndPaint(m_window, &paint);
            pollMessages();
            return;
        }
        
        ::SwapBuffers(m_deviceContext);
        pollMessages();
    }

    void Win32_Window::swapInterval(const uint8_t interval)
    {
        if (m_rtype == RendererType::Hardware)
        {
            PFNWGLSWAPINTERVALEXTPROC wglSwapInterval = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
            wglSwapInterval(interval);
        }
    }

    void Win32_Window::cleanup()
    {
        ::DestroyWindow(m_window);

        if (m_rtype == RendererType::Software)
        {
            if (m_bitmapHandle)
            {
                ::DeleteObject(m_bitmapHandle);
            }
            if (m_deviceContext)
            {
                ::DeleteDC(m_deviceContext);
            }
            ::ClipCursor(nullptr);
    
            delete[] m_backbuffer->backbuffer;
            delete(m_backbuffer);
            m_backbuffer = nullptr;
        }
    }

    void Win32_Window::setTitle(const std::string &title)
    {
        LPCWSTR str = std::wstring(title.begin(), title.end()).c_str();
        ::SetWindowTextW(m_window, str);
    }

    void Win32_Window::setFullscreen(const bool fullscreen)
    {
        if (fullscreen)
        {
            HMONITOR hMonitor = ::MonitorFromWindow(m_window, MONITOR_DEFAULTTONEAREST);
            MONITORINFOEX info;
            info.cbSize = sizeof(MONITORINFOEX);
            DWORD fsStyle = WS_POPUP | WS_VISIBLE;

            ::GetWindowRect(m_window, &m_windowedRect);

            ::GetMonitorInfo(hMonitor, &info);
            ::SetWindowLongPtr(m_window, GWL_STYLE, fsStyle);
            ::SetWindowPos( m_window,
                            NULL, 
                            info.rcMonitor.left, 
                            info.rcMonitor.top, 
                            info.rcMonitor.right, 
                            info.rcMonitor.bottom, 
                            SWP_FRAMECHANGED);
            return;
        }

        ::SetWindowLongPtr(m_window, GWL_STYLE, m_style | WS_VISIBLE);
        ::SetWindowPos( m_window, 
                        NULL, 
                        m_windowedRect.left, 
                        m_windowedRect.top, 
                        m_windowedRect.right - m_windowedRect.left, 
                        m_windowedRect.bottom - m_windowedRect.top, 
                        SWP_FRAMECHANGED);
    }

    void Win32_Window::getPos(int &outX, int &outY)
    {
        ::GetWindowRect(m_window, &m_rect);
        outX = m_rect.left;
        outY = m_rect.top;
    }

    void Win32_Window::getSize(int &outWidth, int & outHeight)
    {
        ::GetWindowRect(m_window, &m_rect);
        outWidth = m_rect.right - m_rect.left;
        outHeight = m_rect.bottom - m_rect.top;
    }

    void Win32_Window::setPos(const int x, const int y)
    {
        ::SetWindowPos( m_window, 
                        NULL, 
                        x, y, 
                        m_rect.right - m_rect.left, 
                        m_rect.bottom - m_rect.top, 
                        SWP_FRAMECHANGED);
    }

    void Win32_Window::setSize(const int width, const int height)
    {
        ::SetWindowPos( m_window,
                        NULL, 
                        m_rect.left, 
                        m_rect.top, 
                        m_rect.left + width, 
                        m_rect.top + height, 
                        SWP_FRAMECHANGED);
    }

    void Win32_Window::setFlash(const UINT count, const DWORD timeout, const bool sound)
    {
        FLASHWINFO info;
        info.cbSize = sizeof(FLASHWINFO);
        info.hwnd = m_window;
        info.dwFlags = FLASHW_ALL;
        info.uCount = count;
        info.dwTimeout = timeout;

        ::FlashWindowEx(&info);
        if (sound)
        {
            ::MessageBeep(MB_OK);
        }
    }

    void Win32_Window::swSetup()
    {
        m_deviceContext = ::CreateCompatibleDC(0);

        m_frameInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        m_frameInfo.bmiHeader.biPlanes = 1;
        m_frameInfo.bmiHeader.biBitCount = 32;
        m_frameInfo.bmiHeader.biCompression = BI_RGB;
        m_backbuffer = new Backbuffer{};
    }

    void Win32_Window::glSetup()
    {
        /* Make dummy context so GLAD can load OpenGL extensions for the ability to create a modern context */
        HDC dummyDC;
        dummyDC = GetDC(m_window);

        PIXELFORMATDESCRIPTOR pfd = { .nSize = sizeof(pfd) };
        ::SetPixelFormat(dummyDC, ChoosePixelFormat(dummyDC, &pfd), &pfd);

        HGLRC dummyRC;
        dummyRC = wglCreateContext(dummyDC);
        ::wglMakeCurrent(dummyDC, dummyRC);

        PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
        PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");

        ::wglMakeCurrent(NULL, NULL);
        ::wglDeleteContext(dummyRC);
        ::ReleaseDC(m_window, dummyDC);
        ::DestroyWindow(m_window);

        /* Create the finished window and make the modern context */
        LPCWSTR cTitle = std::wstring(m_title.begin(), m_title.end()).c_str();
        m_window = ::CreateWindowEx(
            0, 
            className(), 
            cTitle, 
            m_style,
            CW_USEDEFAULT, CW_USEDEFAULT, 
            m_width, m_height, 
            0, 
            0, 
            GetModuleHandle(nullptr), 
            this
        );
        m_deviceContext = ::CreateCompatibleDC(0);
        m_deviceContext = ::GetDC(m_window);

        m_isOpen = m_window;

        int formats = 0;
        UINT numFormats = 0;
        int formatAttribs[] =
        {
            WGL_DRAW_TO_WINDOW_ARB,     TRUE,
            WGL_SUPPORT_OPENGL_ARB,     TRUE,
            WGL_DOUBLE_BUFFER_ARB,      m_hints[DoubleBuffer],
            WGL_SAMPLES_ARB,            m_hints[Samples],
            WGL_ACCELERATION_ARB,       WGL_FULL_ACCELERATION_ARB,
            WGL_PIXEL_TYPE_ARB,         WGL_TYPE_RGBA_ARB,
            WGL_COLOR_BITS_ARB,         m_hints[ColourBits],
            WGL_DEPTH_BITS_ARB,         m_hints[DepthBits],
            WGL_STENCIL_BITS_ARB,       m_hints[StencilBits],
            0
        };

        wglChoosePixelFormatARB(m_deviceContext, formatAttribs, 0, 1, &formats, &numFormats);

        PIXELFORMATDESCRIPTOR desc = { .nSize = sizeof(desc) };
        ::SetPixelFormat(m_deviceContext, formats, &desc);

        int attribList[] =
        { 
            WGL_CONTEXT_MAJOR_VERSION_ARB, m_hints[ContextVerMajor],
            WGL_CONTEXT_MINOR_VERSION_ARB, m_hints[ContextVerMinor],
            WGL_CONTEXT_PROFILE_MASK_ARB,  m_hints[ContextProfile],
            0
        };
        HGLRC hRenderContext = wglCreateContextAttribsARB(m_deviceContext, 0, attribList);

        ::wglMakeCurrent(m_deviceContext, hRenderContext);
    }

    void Win32_Window::registerInputDevices()
    {
        RAWINPUTDEVICE rid[3];

        /* Mouse */
        rid[0].usUsagePage = 0x01;
        rid[0].usUsage = 0x02;
        rid[0].dwFlags = 0;
        rid[0].hwndTarget = m_window;

        /* Keyboard */
        rid[1].usUsagePage = 0x01;
        rid[1].usUsage = 0x06;
        rid[1].dwFlags = 0;
        rid[1].hwndTarget = m_window;

        /* Generic Gamepad */
        rid[2].usUsagePage = 0x01;
        rid[2].usUsage = 0x05;
        rid[2].dwFlags = 0;
        rid[2].hwndTarget = m_window;

        ::RegisterRawInputDevices(rid, 3, sizeof(RAWINPUTDEVICE));
    }

    void Win32_Window::updateCursor()
    {
        if (m_cursorState & CursorFlags::Locked)
        {
            ::SetCursorPos(m_rect.left + (m_width / 2), m_rect.bottom - (m_height / 2));
        }

        if (m_cursorState & CursorFlags::Clamped)
        {
            ::ClipCursor(&m_rect);
        }

        ::ShowCursor(!(m_cursorState & CursorFlags::Hidden));
    }

    void Win32_Window::onWindowResize(const LPARAM lParam)
    {
        m_width  = LOWORD(lParam);
        m_height = HIWORD(lParam);

        if (m_rtype == RendererType::Hardware)
        {
            return;
        }

        if (m_bitmapHandle)
        {
            ::DeleteObject(m_bitmapHandle);
        }

        m_frameInfo.bmiHeader.biWidth  = m_width;
        m_frameInfo.bmiHeader.biHeight = m_height;

        m_bitmapHandle = ::CreateDIBSection(nullptr, &m_frameInfo, DIB_RGB_COLORS, (void**)&m_frontbuffer, nullptr, 0);
        ::SelectObject(m_deviceContext, m_bitmapHandle);
        ::GetWindowRect(m_window, &m_rect);

        if (m_backbuffer)
        {
            m_backbuffer->width  = m_width;
            m_backbuffer->height = m_height;

            delete[] m_backbuffer->backbuffer;
            m_backbuffer->backbuffer = new uint32_t[m_width*m_height];
        }
    }
}