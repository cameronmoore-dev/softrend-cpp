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
        MouseScroll
    };

    Type type = None;

    union
    {
        int keyCode;
        int mouseButton;
        int x, y;
        int scrollDelta;
    };
};