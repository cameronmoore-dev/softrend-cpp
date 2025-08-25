#include "Window.h"

#include <dlgs.h>
#include <minwindef.h>
#include <windows.h>
#include <wingdi.h>
#include <winnt.h>

#include <cstring>
#include <cmath>

Window::~Window()
{
    if (bitmapHandle)
    {
        DeleteObject(bitmapHandle);
    }
    
    if (deviceContext)
    {
        DeleteDC(deviceContext);
    }

    delete[] nextBuffer.pixels;
    delete[] m_depthBuffer;
}

LRESULT Window::handleMessages(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_DESTROY:
        {
            isOpen = false;
        } break;

        case WM_PAINT:
        {
            draw();
            return 0;
        }break;

        case WM_SIZE:
        {
            frameInfo.bmiHeader.biWidth = LOWORD(lParam);
            frameInfo.bmiHeader.biHeight = HIWORD(lParam);

            if (bitmapHandle)
            {
                DeleteObject(bitmapHandle);
                delete[] nextBuffer.pixels;
                delete[] m_depthBuffer;
            }

            bitmapHandle = CreateDIBSection(nullptr, &frameInfo, DIB_RGB_COLORS, (void**)&currentBuffer.pixels, nullptr, 0);
            SelectObject(deviceContext, bitmapHandle);

            currentBuffer.width = LOWORD(lParam);
            currentBuffer.height = HIWORD(lParam);

            nextBuffer.pixels = new UINT[LOWORD(lParam) * HIWORD(lParam)];
            nextBuffer.width = LOWORD(lParam);
            nextBuffer.height = HIWORD(lParam);

            m_depthBuffer = new FLOAT[LOWORD(lParam) * HIWORD(lParam)];

        } break;

        default:
            return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
    }

    return TRUE;
}

void Window::setup()
{
    frameInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    frameInfo.bmiHeader.biPlanes = 1;
    frameInfo.bmiHeader.biBitCount = 32;
    frameInfo.bmiHeader.biCompression = BI_RGB;
    deviceContext = CreateCompatibleDC(0);
}

void Window::clear()
{
    InvalidateRect(m_hWnd, nullptr, FALSE);
    std::fill_n(nextBuffer.pixels, nextBuffer.width*nextBuffer.height, 0x000000);
    std::fill_n(m_depthBuffer, nextBuffer.width*nextBuffer.height, INFINITY);
}

void Window::draw()
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(m_hWnd, &ps);

    // nextBuffer.pixels[(p++) % (nextBuffer.width*nextBuffer.height)] = Utils::pcgHash(rand());
    // nextBuffer.pixels[Utils::pcgHash(rand()) % (nextBuffer.width*nextBuffer.height)] = 0;

    // for (UINT i = 0; i < nextBuffer.width*nextBuffer.height; i++)
    // {
    //     nextBuffer.pixels[i] = Utils::pcgHash(rand());
    // }

    // tri.draw(nextBuffer);
    cube.draw(nextBuffer, m_depthBuffer);

    nextBuffer.pixels[currentBuffer.width/2 + currentBuffer.height/2 * currentBuffer.width] = 0xff00ff;

    std::memcpy(currentBuffer.pixels, nextBuffer.pixels, nextBuffer.width*nextBuffer.height*sizeof(UINT));
    BitBlt(hdc, 
        ps.rcPaint.left, ps.rcPaint.top, 
        currentBuffer.width, currentBuffer.height, 
        deviceContext, 
        ps.rcPaint.left, ps.rcPaint.top, 
        SRCCOPY);
    
    EndPaint(m_hWnd, &ps);
}