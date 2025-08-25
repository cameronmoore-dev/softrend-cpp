#pragma once

#include "WindowBase.h"
#include "Structs.h"
#include "Utils.h"
#include "Triangle.h"
#include "Model.h"

class Window : public WindowBase<Window>
{
    
public:
    ~Window();

    LPCSTR className() const { return "WindowClassName"; }
    LRESULT handleMessages(UINT uMsg, WPARAM wParam, LPARAM lParam);

    void setup();
    void clear();
    void draw();

private:
    BITMAPINFO frameInfo;
    HBITMAP bitmapHandle = 0;
    HDC deviceContext = 0;

    INT p = 0;

    Triangle tri{{-160, 120, -1.f, 0xff0000},
                 {0, -120, -1.f, 0x00ff00},
                 {160, 120, -1.f, 0x0000ff}};

    Model cube = Model("./resources/cube.obj");

    FrameBuffer currentBuffer;
    FrameBuffer nextBuffer;

private:
};