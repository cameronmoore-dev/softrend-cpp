#include "os/window.h"

#include <windows.h>
#include <windowsx.h>

#include <stdlib.h>

#include <stdio.h>

#ifndef UNICODE
#define UNICODE 1
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

typedef struct platform_context
{
    HWND        hwnd;
    BITMAPINFO  bitmap_info;
    HBITMAP     bitmap;
    HDC         device_context;
    RECT        rect;
    RECT        windowed_rect;
} platform_context;

void update_cursor(window_ *window);
void window_on_resize(window_ *window, LPARAM lparam);
LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

window_ *window_create(const char *title, u32 width, u32 height)
{
    timeBeginPeriod(1);

    const char CLASS_NAME[] = { "SoftRendWindowClass" };
    WNDCLASS wc = 
    {
        .lpfnWndProc = window_proc,
        .hInstance = GetModuleHandle(NULL),
        .lpszClassName = CLASS_NAME,
        .hCursor = LoadCursor(NULL, IDC_ARROW)
    };
    RegisterClass(&wc);

    window_ *window = (window_ *)malloc(sizeof(window_));
    window->platform = (platform_context *)malloc(sizeof(platform_context));
    window->frontbuffer = (u32 *)malloc(width * height * sizeof(u32));
    window->title = title;
    window->width = width;
    window->height = height;
    window->backbuffer.data = (u32 *)malloc(width * height * sizeof(u32));
    window->backbuffer.width = width;
    window->backbuffer.height = height;
    window->isOpen = true;
    window->input = (input_){};
    window->frame_start = (timepoint_){};

    window->platform->hwnd = CreateWindowEx(
        0, CLASS_NAME,
        title, 
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        width, height, 
        NULL, NULL, 
        GetModuleHandle(NULL), NULL
    );

    SetProp(window->platform->hwnd, "SoftRendWindow", (HANDLE)window);

    // Bitmap setup
    window->platform->device_context = CreateCompatibleDC(0);
    window->platform->bitmap_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    window->platform->bitmap_info.bmiHeader.biPlanes = 1;
    window->platform->bitmap_info.bmiHeader.biBitCount = 32;
    window->platform->bitmap_info.bmiHeader.biCompression = BI_RGB;

    // Register mouse as raw input device
    RAWINPUTDEVICE rid[1];
    rid[0].usUsagePage = 0x01;
    rid[0].usUsage = 0x02;
    rid[0].dwFlags = 0;
    rid[0].hwndTarget = window->platform->hwnd;
    RegisterRawInputDevices(rid, 1, sizeof(RAWINPUTDEVICE));

    ShowWindow(window->platform->hwnd, SW_NORMAL);
    SetFocus(window->platform->hwnd);

    return window;
}

void window_swap_buffers(window_ *window)
{
    update_cursor(window);

    InvalidateRect(window->platform->hwnd, NULL, false);

    PAINTSTRUCT paint;
    HDC hdc = BeginPaint(window->platform->hwnd, &paint);

    u32 buf_size = window->backbuffer.width * window->backbuffer.height * sizeof(u32);
    memcpy(window->frontbuffer, window->backbuffer.data, buf_size);
    BitBlt(hdc, 
        paint.rcPaint.left, paint.rcPaint.top, 
        window->backbuffer.width, window->backbuffer.height, 
        window->platform->device_context, 
        paint.rcPaint.left, paint.rcPaint.top, 
        SRCCOPY);

    EndPaint(window->platform->hwnd, &paint);
}

void window_sleep(window_ *window)
{
    duration_ target_frametime = set_duration(1.0f / 60.f);
    timepoint_ next_frame_timepoint = get_future_point(&window->frame_start, &target_frametime);
    timepoint_ now = get_time();
    duration_ diff = timepoint_diff(&next_frame_timepoint, &now);

    // NOTE: When using the window's close button
    //       the diff can be a 'negative' number (underflowed to a very large number),
    //       so we have to check if the signed diff is greater than than 0
    if ((s64)diff.ms > 0)
    {
        Sleep(((DWORD)diff.ms));
    }

    window->frame_start = get_time();
}

void window_cleanup(window_ *window)
{
    timeEndPeriod(1);

    DestroyWindow(window->platform->hwnd);
    DeleteObject(window->platform->bitmap);
    DeleteDC(window->platform->device_context);

    free(window->backbuffer.data);
    free(window->platform);
    free(window);
}

void update_cursor(window_ *window)
{
    RECT rect;
    GetWindowRect(window->platform->hwnd, &rect);
    SetCursorPos(rect.left + (window->width / 2), rect.bottom - (window->height / 2));
    ShowCursor(false);
}

bool window_is_open(window_ *window)
{
    return window->isOpen;
}

void window_close(window_ *window)
{
    window->isOpen = false;
}

void window_pump_messages(window_ *window)
{
    MSG msg = {};
    while (PeekMessage(&msg, window->platform->hwnd, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void window_on_resize(window_ *window, LPARAM lparam)
{
    if (window->width  == LOWORD(lparam) && 
        window->height == HIWORD(lparam)) return;

    window->width  = LOWORD(lparam);
    window->height = HIWORD(lparam);

    if (window->platform->bitmap)
    {
        DeleteObject(window->platform->bitmap);
    }

    window->platform->bitmap_info.bmiHeader.biWidth  = window->width;
    window->platform->bitmap_info.bmiHeader.biHeight = window->height;

    // NOTE: When a new DIBSection is created, the window's frontbuffer is automatically reallocated.
    //       Don't free or malloc
    window->platform->bitmap = CreateDIBSection( NULL, 
                                        &window->platform->bitmap_info, 
                                        DIB_RGB_COLORS, 
                                        (void**)&window->frontbuffer, 
                                        NULL, 0);
    SelectObject(window->platform->device_context, window->platform->bitmap);

    // TODO: Use realloc()
    if (window->backbuffer.data)   free(window->backbuffer.data);
    window->backbuffer.data = (u32 *)malloc(window->width * window->height * sizeof(u32));
    window->backbuffer.width  = window->width;
    window->backbuffer.height = window->height;
}

LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    window_ *window  = GetProp(hwnd, "SoftRendWindow");
    if (!window)
    {
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }

    switch (msg)
    {
        case WM_CLOSE:
        {
            window_close(window);
            
        } break;

        case WM_SIZE:
        {
            window_on_resize(window, lparam);
        } break;

        case WM_KEYDOWN:
        {
            _set_key_pressed(&window->input, (u32)wparam);
        } break;

        case WM_KEYUP:
        {
            _set_key_released(&window->input, (u32)wparam);
        } break;

        case WM_INPUT:
        {
            UINT size;
            GetRawInputData((HRAWINPUT)lparam, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));

            BYTE input_buffer[48];
            if (GetRawInputData((HRAWINPUT)lparam, RID_INPUT, input_buffer, &size, sizeof(RAWINPUTHEADER)) != size)
            {
                break;
            }

            RAWINPUT *raw = (RAWINPUT *)input_buffer;
            if (raw->header.dwType == RIM_TYPEMOUSE)
            {
                if (raw->data.mouse.usFlags == MOUSE_MOVE_RELATIVE)
                {
                    window->input.mouse_dx +=  raw->data.mouse.lLastX;
                    window->input.mouse_dy += -raw->data.mouse.lLastY;
                }
            }
        } break;

        case WM_DESTROY:
        {
            RemoveProp(hwnd, "SoftRendWindow");
            PostQuitMessage(0);
        } break;

        default:
        {
            return DefWindowProc(hwnd, msg, wparam, lparam);
        } break;
    }

    return DefWindowProc(hwnd, msg, wparam, lparam);
}