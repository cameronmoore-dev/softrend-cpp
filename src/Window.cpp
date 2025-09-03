#include "Window.h"

#include <windowsx.h>
#include <cstring>
#include <cmath>

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

        case WM_KEYDOWN:
        {
            Event e;
            e.type = Event::KeyPressed;
            e.keyCode = wParam;
            m_events.push(e);
        } break;

        case WM_KEYUP:
        {
            Event e;
            e.type = Event::KeyReleased;
            e.keyCode = wParam;
            m_events.push(e);
        }break;

        case WM_LBUTTONDOWN:
        {
            Event e;
            e.type = Event::MousePressed;
            e.mouseButton = 0;
            m_events.push(e);
        } break;

        case WM_LBUTTONUP:
        {
            Event e;
            e.type = Event::MouseReleased;
            e.mouseButton = 0;
            m_events.push(e);
        } break;

        case WM_RBUTTONDOWN:
        {
            Event e;
            e.type = Event::MousePressed;
            e.mouseButton = 1;
            m_events.push(e);
        } break;

        case WM_RBUTTONUP:
        {
            Event e;
            e.type = Event::MouseReleased;
            e.mouseButton = 1;
            m_events.push(e);
        } break;

        case WM_MOUSEMOVE:
        {
            Event e;
            e.type = Event::MouseMove;
            e.x = GET_X_LPARAM(lParam);
            e.y = GET_Y_LPARAM(lParam);
            m_events.push(e);
        } break;

        case WM_MOUSEWHEEL:
        {
            Event e;
            e.type = Event::MouseScroll;
            e.scrollDelta = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
            m_events.push(e);
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
        ShowWindow(m_hwnd, 1);

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

void Window::draw(std::vector<Vertex>& vertices, Image* texture, const Mat4& model, const Mat4& view, const Mat4& projection)
{
    std::vector<Renderer::RasterVertex> rvs;
    for (Vertex& v : vertices)
    {
        rvs.emplace_back(m_renderer.calculateScreenSpace(v, model, view, projection, m_width, m_height));
    }

    for (uint32_t i = 0; i < rvs.size(); i+=3)
    {
        m_renderer.rasterise(m_backBuffer, m_depthBuffer, 1.f, rvs[i], rvs[i+1], rvs[i+2], m_width, m_height, texture);
    }
}

void Window::display()
{
    PAINTSTRUCT paint;
    HDC hdc = BeginPaint(m_hwnd, &paint);

    // nextBuffer.pixels[(p++) % (nextBuffer.width*nextBuffer.height)] = Utils::pcgHash(rand());
    // nextBuffer.pixels[Utils::pcgHash(rand()) % (nextBuffer.width*nextBuffer.height)] = 0;

    // for (UINT i = 0; i < nextBuffer.width*nextBuffer.height; i++)
    // {
    //     nextBuffer.pixels[i] = Utils::pcgHash(rand());
    // }

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
    m_depthBuffer.resize(m_width*m_height);
}