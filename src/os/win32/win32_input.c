#include "../input.h"

#include <windows.h>

key get_platform_key(u32 keycode);

bool is_key_pressed(input_ *input, key key)
{
    bool pressed = input->inputs[key];
    input->inputs[key] = false;

    return pressed;
}

bool is_key_held(input_ *input, key key)
{
    return input->inputs[key];
}

void _set_key_pressed(input_ *input, u32 keycode)
{
    key key = get_platform_key(keycode);
    if (key != INVALID)
    {
        input->inputs[key] = true;
    }
}

void _set_key_released(input_ *input, u32 keycode)
{
    key key = get_platform_key(keycode);
    if (key != INVALID)
    {
        input->inputs[key] = false;
    }
}


key get_platform_key(u32 keycode)
{
    key key = INVALID;
    switch (keycode)
    {
        case '1': key = NUM1; break;
        case '2': key = NUM2; break;
        case '3': key = NUM3; break;
        case '4': key = NUM4; break;
        case '5': key = NUM5; break;
        case '6': key = NUM6; break;
        case 'Q': key = KEY_Q; break;
        case 'W': key = KEY_W; break;
        case 'E': key = KEY_E; break;
        case 'A': key = KEY_A; break;
        case 'S': key = KEY_S; break;
        case 'D': key = KEY_D; break;
        case VK_ESCAPE: key = ESC; break;

        default: break;
    }

    return key;
}