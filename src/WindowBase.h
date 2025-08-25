#pragma once

#include <windows.h>

template<class T>
class WindowBase
{
public:
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        T* pThis = nullptr;
        if (uMsg == WM_CREATE)
        {
            CREATESTRUCT* create = (CREATESTRUCT*)lParam;
            pThis = (T*)create->lpCreateParams;
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);
            pThis->m_hWnd = hWnd;
        }
        else
        {
            pThis = (T*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        }
        if (pThis)
        {
            return pThis->handleMessages(uMsg, wParam, lParam);
        }
        else
        {
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
    }

    INT create( LPCSTR lpWindowName, 
                DWORD dwStyle,
                DWORD dwExStyle = 0, 
                INT x = CW_USEDEFAULT, INT y = CW_USEDEFAULT, 
                INT width = 640, INT height = 480, 
                HWND hwndParent = 0, HMENU hMenu = 0 )
    {
        WNDCLASS wc = {};
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = T::WindowProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = className();

        RegisterClass(&wc);

        m_hWnd = CreateWindowEx(
            dwExStyle, 
            className(), 
            lpWindowName, 
            dwStyle, 
            x, y, 
            width, height, 
            hwndParent, 
            hMenu, 
            GetModuleHandle(nullptr), 
            this
        );

        isOpen = m_hWnd;
        return m_hWnd ? TRUE : FALSE;
    }

    HWND handle() { return m_hWnd; }
    bool isOpen = false;

protected:
    HWND m_hWnd;

protected:
    virtual LPCSTR className() const = 0;
    virtual LRESULT handleMessages(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;

    virtual void setup() = 0;
    virtual void clear() = 0;
    virtual void draw() = 0;
};