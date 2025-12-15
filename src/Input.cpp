#include "Input.h"

#include <cmath>
#include <algorithm>

#include <stdio.h>

/* https://www.psdevwiki.com/ps4/DS4-USB#Data_Format */

#define XBOX_VID            0x45E
#define PS4_VID             0x54C
#define SWITCH_PRO_VID      0x57E

bool Input::isGamepadConnected(uint8_t cid)
{
    return cid < gamepads.size();
}

bool Input::isGamepadButtonPressed(uint8_t cid, JAWL_GAMEPAD_BUTTONS button)
{
    if (cid >= gamepads.size())
    {
        printf("no connected gamepad matches this id: %d\n", cid);
        return -1;
    }

    return ((gamepads[cid].buttons >> button) & 0x1);
}

float Input::getControllerAxisValue(uint8_t cid, JAWL_GAMEPAD_AXIS axis)
{
    if (cid >= gamepads.size())
    {
        printf("no connected gamepad matches this id: %d\n", cid);
        return -1;
    }

    JAWL_Gamepad &gamepad = gamepads[cid];
    float axisValue = (float)gamepad.stickAxes[axis];
    float whole = (float)gamepad.maxAxisValue;
    float axisPercent = std::floor(axisValue / whole * 100.f);

    if (std::abs(axisPercent) > JAWL_LEFTSTICK_DEADZONE_PERCENT)
    {
        /* Normalise value between -1.0 and 1.0 */
        return (axisValue / whole);
    }
    
    return 0.f;
}

void Input::pollGamepads(RAWINPUT *raw, Event &event)
{
    int numXInputDevices = 0;
    for (int i = 0; i < gamepads.size(); i++)
    {
        JAWL_Gamepad &gamepad = gamepads[i];
        switch (gamepad.vid)
        {
            case XBOX_VID:
            {
                XINPUT_STATE state;
                ZeroMemory(&state, sizeof(XINPUT_STATE));
        
                XInputGetState(numXInputDevices, &state);
                xboxToJAWL(gamepad, state);

                numXInputDevices++;
            } break;
    
            case PS4_VID:
            {
                dualshockToJAWL(gamepad, raw->data.hid.bRawData);
    
                /* The 4th bit is set if none of the dpad buttons are pressed.
                 * So this just checks to see if only that bit is set, if it is, then no other are buttons are being pressed
                 */
                if (gamepad.buttons != 0x8)
                {
                    event.type = Event::GamepadButton;
                }
            } break;
    
            default: break;
        }
    }
}

void Input::xboxToJAWL(JAWL_Gamepad &gamepad, XINPUT_STATE &state)
{
    gamepad.buttons = state.Gamepad.wButtons;
    gamepad.stickAxes[JAWL_GAMEPAD_AXIS::LX] = state.Gamepad.sThumbLX;
    gamepad.stickAxes[JAWL_GAMEPAD_AXIS::LY] = state.Gamepad.sThumbLY;
    gamepad.stickAxes[JAWL_GAMEPAD_AXIS::RX] = state.Gamepad.sThumbRX;
    gamepad.stickAxes[JAWL_GAMEPAD_AXIS::RY] = state.Gamepad.sThumbRY;
    gamepad.triggers = ((state.Gamepad.bLeftTrigger << 8) | state.Gamepad.bRightTrigger);
    gamepad.maxAxisValue = INT16_MAX;
}

void Input::dualshockToJAWL(JAWL_Gamepad &gamepad, BYTE *rawData)
{
    gamepad.stickAxes[JAWL_GAMEPAD_AXIS::LX] = rawData[1] - 128;
    gamepad.stickAxes[JAWL_GAMEPAD_AXIS::LY] = -(rawData[2] - 128);
    gamepad.stickAxes[JAWL_GAMEPAD_AXIS::RX] = rawData[3] - 128;
    gamepad.stickAxes[JAWL_GAMEPAD_AXIS::RY] = -(rawData[4] - 128);

    uint16_t buttonsRemap = 0;
    buttonsRemap ^= (((buttonsRemap >> DPAD_UP) ^ (rawData[5] >> 0)) & 0x1) << DPAD_UP;
    buttonsRemap ^= (((buttonsRemap >> DPAD_DOWN) ^ (rawData[5] >> 1)) & 0x1) << DPAD_DOWN;
    buttonsRemap ^= (((buttonsRemap >> DPAD_LEFT) ^ (rawData[5] >> 2)) & 0x1) << DPAD_LEFT;
    buttonsRemap ^= (((buttonsRemap >> DPAD_RIGHT) ^ (rawData[5] >> 3)) & 0x1) << DPAD_RIGHT;
    buttonsRemap ^= (((buttonsRemap >> START) ^ (rawData[6] >> 5)) & 0x1) << START;
    buttonsRemap ^= (((buttonsRemap >> BACK) ^ (rawData[6] >> 4)) & 0x1) << BACK;
    buttonsRemap ^= (((buttonsRemap >> LEFT_STICK) ^ (rawData[6] >> 6)) & 0x1) << LEFT_STICK;
    buttonsRemap ^= (((buttonsRemap >> RIGHT_STICK) ^ (rawData[6] >> 7)) & 0x1) << RIGHT_STICK;
    buttonsRemap ^= (((buttonsRemap >> LEFT_SHOULDER) ^ (rawData[6] >> 0)) & 0x1) << LEFT_SHOULDER;
    buttonsRemap ^= (((buttonsRemap >> RIGHT_SHOULDER) ^ (rawData[6] >> 1)) & 0x1) << RIGHT_SHOULDER;
    buttonsRemap ^= (((buttonsRemap >> A) ^ (rawData[5] >> 5)) & 0x1) << A;
    buttonsRemap ^= (((buttonsRemap >> B) ^ (rawData[5] >> 6)) & 0x1) << B;
    buttonsRemap ^= (((buttonsRemap >> X) ^ (rawData[5] >> 4)) & 0x1) << X;
    buttonsRemap ^= (((buttonsRemap >> Y) ^ (rawData[5] >> 7)) & 0x1) << Y;

    gamepad.buttons = buttonsRemap;
    gamepad.triggers = ((rawData[8] << 8) | rawData[9]);
    gamepad.maxAxisValue = INT8_MAX;
}

void Input::storeGameControllers()
{
    UINT size = 0;
    GetRawInputDeviceList(nullptr, &size, sizeof(RAWINPUTDEVICELIST));

    if (!size)
    {
        printf("no devices were found!\n");
        return;
    }

    gamepads.clear();
    RAWINPUTDEVICELIST devices[size];
    GetRawInputDeviceList(devices, &size, sizeof(RAWINPUTDEVICELIST));

    for (int i = 0; i < size; i++)
    {
        if (devices[i].dwType == RIM_TYPEHID)
        {
            RID_DEVICE_INFO info;
            GetRawInputDeviceInfoA(devices[i].hDevice, RIDI_DEVICEINFO, nullptr, &size);
            GetRawInputDeviceInfoA(devices[i].hDevice, RIDI_DEVICEINFO, &info, &size);

            /* Is this device a gamepad */
            if (info.hid.usUsage == 0x5)
            {
                JAWL_Gamepad pad {};
                pad.vid = info.hid.dwVendorId;
                gamepads.emplace_back(pad);
                printf("Gamepad Connected (Device: %d): %#x\n", i, pad.vid);
            }
        }
    }
}