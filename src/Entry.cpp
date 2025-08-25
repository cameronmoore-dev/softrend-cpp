#include <windef.h>
#include <windows.h>
#include <wingdi.h>
#include <iostream>

#include <chrono>

#include "Window.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    srand(static_cast<unsigned int>(time(0)));

    Window win;
    if (!win.create("Software Renderer", WS_OVERLAPPEDWINDOW))
    {
        std::cerr << "ERROR::WINDOW::failed to create!" << std::endl;
        return 1;
    }

    win.setup();

    ShowWindow(win.handle(), nShowCmd);

    while (win.isOpen)
    {
        auto start = std::chrono::high_resolution_clock::now();

        MSG msg = {};
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            // TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        win.clear();

        // draw frame buffer by sending WM_PAINT message
        UpdateWindow(win.handle());

        auto end = std::chrono::high_resolution_clock::now();
        double duration = std::chrono::duration<double>(end-start).count();

        printf("%.1f\n", 1.f/duration);
    }

    return 0;
}