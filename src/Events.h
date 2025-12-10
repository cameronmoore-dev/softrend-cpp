#pragma once

struct Event
{
    enum Type
    {
        None,
        KeyPressed,
        KeyReleased,
        MousePressed,
        MouseReleased,
        MouseMove,
        MouseScroll,
        Gamepad
    };

    Type type = None;

    union
    {
        int keyCode;
        int mouseButton;
        struct
        {
            int x;
            int y;
        } mousePos;
        
        int scrollDelta;
        BYTE gamepadData[13];
    };
};