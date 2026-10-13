#pragma once

#include "typedefs.h"

typedef enum key
{
    INVALID,
    NUM1,
    NUM2,
    NUM3,
    NUM4,
    NUM5,
    NUM6,
    KEY_Q, KEY_W, KEY_E,
    KEY_A, KEY_S, KEY_D,
    ESC,
    COUNT
} key;

typedef struct input
{
    bool inputs[COUNT];
    u32 mx;
    u32 my;
    s32 mouse_dx;
    s32 mouse_dy;
} input_;

bool is_key_pressed(input_ *input, key key);
bool is_key_held(input_ *input, key key);
void _set_key_pressed(input_ *input, u32 keycode);
void _set_key_released(input_ *input, u32 keycode);