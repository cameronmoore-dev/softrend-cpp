#pragma once

#ifndef UNICODE
#define UNICODE 1
#endif

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
    #define NOMINMAX
#endif

#include <glad/wgl.h>

#include "../base_window.h"
#include "win32_input.h"

namespace jawl
{
    class Win32_Window : public Window
    {
    public:
        Win32_Window() = default;
        ~Win32_Window() override {}

        Win32_Window *create(const std::string &title, uint16_t width, uint16_t height, uint32_t style, RendererType rtype) override;
        void cleanup() override;
        void swapBuffers() override;
        void swapInterval(uint8_t interval) override;
    
        void setTitle(const std::string &title) override;
        void setFullscreen(bool fullscreen) override;
        void getPos(int &outX, int &outY) override;
        void getSize(int &outWidth, int &outHeight) override;
        void getScreenSize(int &outWidth, int &outHeight) override;
        void setPos(int x, int y) override;
        void setSize(int width, int height) override;
        void setFlash(UINT count, DWORD timeout, bool sound);
        
        bool isFocused() override { return GetFocus() == m_window; }
    
    private:
        HWND        m_window;
        BITMAPINFO  m_frameInfo;
        HBITMAP     m_bitmapHandle;
        HDC         m_deviceContext;
        RECT        m_rect;
        RECT        m_windowedRect;

        std::vector<BYTE> m_inputBuffer;
    
    private:
        static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
        {
            Win32_Window* wnd = nullptr;
            if (uMsg == WM_CREATE)
            {
                CREATESTRUCT* create = (CREATESTRUCT*)lParam;
                wnd = (Win32_Window*)create->lpCreateParams;
                SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)wnd);
                wnd->m_window = hwnd;
            }
            else
            {
                wnd = (Win32_Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
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
        
        LPCWSTR className() const { return L"JawlWindowClass"; }
        LRESULT handleMessages(UINT uMsg, WPARAM wParam, LPARAM lParam);
        void registerWinClass();
        void pollMessages();

        void swSetup() override;
        void glSetup() override;
        void registerInputDevices() override;
        void updateCursor() override;
    
        void onWindowResize(LPARAM lParam);
    };
}