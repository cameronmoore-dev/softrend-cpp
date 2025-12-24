#pragma once

namespace jawl
{
    struct Event
    {
        enum Type
        {
            None = 0,
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

            WindowResize,
            DeviceChanged
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

            struct
            {
                int width;
                int height;
            } wndResizeInfo;
        };
    };
}