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
        GamepadButtonPressed,
        GamepadButtonReleased,
        GamepadAxisMoved,
        GamepadTrigger,
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

        int gamepadButton; 
        struct
        {
            int axis;
            int value;
        } axisInfo;

        struct
        {
            int trigger;
            int value;
        } triggerInfo;
    };
};