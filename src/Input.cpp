#include "Input.h"

#include <cmath>
#include <algorithm>

#include <stdio.h>

/* https://www.psdevwiki.com/ps4/DS4-USB#Data_Format */

#define XBOX_VID            0x45E
#define PS4_VID             0x54C
#define SWITCH_PRO_VID      0x57E

bool Input::isKeyPressed(JAWL_KEY key)
{
    return ((GetAsyncKeyState(jawlToVK(key)) & 0x8000) != 0);
}

uint16_t Input::getKey(JAWL_KEY key)
{
    return (uint16_t)jawlToVK(key);
}

uint16_t Input::jawlToVK(JAWL_KEY key)
{
    switch (key)
    {
        case JAWL_KEY::A:          return 'A';
        case JAWL_KEY::B:          return 'B';
        case JAWL_KEY::C:          return 'C';
        case JAWL_KEY::D:          return 'D';
        case JAWL_KEY::E:          return 'E';
        case JAWL_KEY::F:          return 'F';
        case JAWL_KEY::G:          return 'G';
        case JAWL_KEY::H:          return 'H';
        case JAWL_KEY::I:          return 'I';
        case JAWL_KEY::J:          return 'J';
        case JAWL_KEY::K:          return 'K';
        case JAWL_KEY::L:          return 'L';
        case JAWL_KEY::M:          return 'M';
        case JAWL_KEY::N:          return 'N';
        case JAWL_KEY::O:          return 'O';
        case JAWL_KEY::P:          return 'P';
        case JAWL_KEY::Q:          return 'Q';
        case JAWL_KEY::R:          return 'R';
        case JAWL_KEY::S:          return 'S';
        case JAWL_KEY::T:          return 'T';
        case JAWL_KEY::U:          return 'U';
        case JAWL_KEY::V:          return 'V';
        case JAWL_KEY::W:          return 'W';
        case JAWL_KEY::X:          return 'X';
        case JAWL_KEY::Y:          return 'Y';
        case JAWL_KEY::Z:          return 'Z';
        case JAWL_KEY::Num0:       return '0';
        case JAWL_KEY::Num1:       return '1';
        case JAWL_KEY::Num2:       return '2';
        case JAWL_KEY::Num3:       return '3';
        case JAWL_KEY::Num4:       return '4';
        case JAWL_KEY::Num5:       return '5';
        case JAWL_KEY::Num6:       return '6';
        case JAWL_KEY::Num7:       return '7';
        case JAWL_KEY::Num8:       return '8';
        case JAWL_KEY::Num9:       return '9';
        case JAWL_KEY::Escape:     return VK_ESCAPE;
        case JAWL_KEY::LControl:   return VK_LCONTROL;
        case JAWL_KEY::LShift:     return VK_LSHIFT;
        case JAWL_KEY::LAlt:       return VK_LMENU;
        case JAWL_KEY::LSystem:    return VK_LWIN;
        case JAWL_KEY::RControl:   return VK_RCONTROL;
        case JAWL_KEY::RShift:     return VK_RSHIFT;
        case JAWL_KEY::RAlt:       return VK_RMENU;
        case JAWL_KEY::RSystem:    return VK_RWIN;
        case JAWL_KEY::Menu:       return VK_APPS;
        case JAWL_KEY::LBracket:   return VK_OEM_4;
        case JAWL_KEY::RBracket:   return VK_OEM_6;
        case JAWL_KEY::Semicolon:  return VK_OEM_1;
        case JAWL_KEY::Comma:      return VK_OEM_COMMA;
        case JAWL_KEY::Period:     return VK_OEM_PERIOD;
        case JAWL_KEY::Apostrophe: return VK_OEM_7;
        case JAWL_KEY::Slash:      return VK_OEM_2;
        case JAWL_KEY::Backslash:  return VK_OEM_5;
        case JAWL_KEY::Grave:      return VK_OEM_3;
        case JAWL_KEY::Equal:      return VK_OEM_PLUS;
        case JAWL_KEY::Hyphen:     return VK_OEM_MINUS;
        case JAWL_KEY::Space:      return VK_SPACE;
        case JAWL_KEY::Enter:      return VK_RETURN;
        case JAWL_KEY::Backspace:  return VK_BACK;
        case JAWL_KEY::Tab:        return VK_TAB;
        case JAWL_KEY::PageUp:     return VK_PRIOR;
        case JAWL_KEY::PageDown:   return VK_NEXT;
        case JAWL_KEY::End:        return VK_END;
        case JAWL_KEY::Home:       return VK_HOME;
        case JAWL_KEY::Insert:     return VK_INSERT;
        case JAWL_KEY::Delete:     return VK_DELETE;
        case JAWL_KEY::Add:        return VK_ADD;
        case JAWL_KEY::Subtract:   return VK_SUBTRACT;
        case JAWL_KEY::Multiply:   return VK_MULTIPLY;
        case JAWL_KEY::Divide:     return VK_DIVIDE;
        case JAWL_KEY::Left:       return VK_LEFT;
        case JAWL_KEY::Right:      return VK_RIGHT;
        case JAWL_KEY::Up:         return VK_UP;
        case JAWL_KEY::Down:       return VK_DOWN;
        case JAWL_KEY::Numpad0:    return VK_NUMPAD0;
        case JAWL_KEY::Numpad1:    return VK_NUMPAD1;
        case JAWL_KEY::Numpad2:    return VK_NUMPAD2;
        case JAWL_KEY::Numpad3:    return VK_NUMPAD3;
        case JAWL_KEY::Numpad4:    return VK_NUMPAD4;
        case JAWL_KEY::Numpad5:    return VK_NUMPAD5;
        case JAWL_KEY::Numpad6:    return VK_NUMPAD6;
        case JAWL_KEY::Numpad7:    return VK_NUMPAD7;
        case JAWL_KEY::Numpad8:    return VK_NUMPAD8;
        case JAWL_KEY::Numpad9:    return VK_NUMPAD9;
        case JAWL_KEY::F1:         return VK_F1;
        case JAWL_KEY::F2:         return VK_F2;
        case JAWL_KEY::F3:         return VK_F3;
        case JAWL_KEY::F4:         return VK_F4;
        case JAWL_KEY::F5:         return VK_F5;
        case JAWL_KEY::F6:         return VK_F6;
        case JAWL_KEY::F7:         return VK_F7;
        case JAWL_KEY::F8:         return VK_F8;
        case JAWL_KEY::F9:         return VK_F9;
        case JAWL_KEY::F10:        return VK_F10;
        case JAWL_KEY::F11:        return VK_F11;
        case JAWL_KEY::F12:        return VK_F12;
        case JAWL_KEY::F13:        return VK_F13;
        case JAWL_KEY::F14:        return VK_F14;
        case JAWL_KEY::F15:        return VK_F15;
        case JAWL_KEY::Pause:      return VK_PAUSE;
        default:                   return 0;
    }
}


bool Input::isGamepadConnected(uint8_t slot)
{
    return slot < gamepads.size();
}

bool Input::isGamepadButtonPressed(uint8_t slot, JAWL_GAMEPAD_BUTTONS button)
{
    if (slot >= gamepads.size())
    {
        printf("no connected gamepad matches this id: %d\n", slot);
        return false;
    }

    return ((gamepads[slot].buttons >> button) & 0x1);
}

float Input::controllerAxisValue(uint8_t slot, JAWL_GAMEPAD_AXIS axis)
{
    if (slot >= gamepads.size())
    {
        printf("no connected gamepad matches this id: %d\n", slot);
        return -1.f;
    }

    JAWL_Gamepad &gamepad = gamepads[slot];
    float axisValue = (float)gamepad.stickAxes[axis];
    float whole = (float)gamepad.maxAxisValue;
    float axisPercent = std::floor(axisValue / whole * 100.f);

    if (std::abs(axisPercent) > JAWL_JOYSTICK_DEADZONE_PERCENT)
    {
        /* Normalise value between -1.0 and 1.0 */
        return (axisValue / whole);
    }
    
    return 0.f;
}

void Input::pollGamepads(RAWHID &hidData, std::queue<Event> &events)
{
    int numXInputDevices = 0;
    for (uint8_t i = 0; i < gamepads.size(); i++)
    {
        JAWL_Gamepad &gamepad = gamepads[i];
        JAWL_Gamepad &previousState = previousGamepadStates[i];

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
                dualshockToJAWL(gamepad, hidData.bRawData);
            } break;
            
            default: break;
        }
        
        compareGamepadStates(gamepad, previousState, events);
        previousState = gamepad;
    }
}

void Input::compareGamepadStates(JAWL_Gamepad &current, JAWL_Gamepad &previous, std::queue<Event> &events)
{
    if (current.buttons == previous.buttons)
    {
        goto compareAxes;
    }

    for (uint8_t i = 0; i < JAWL_GAMEPAD_BUTTONS::TOTAL_BUTTONS; i++)
    {
        if (((current.buttons >> i) ^ (previous.buttons >> i)) == 1)
        {
            Event e;
            if ((current.buttons >> i) & 0x1)
            {
                e.type = Event::GamepadButtonPressed;
            }
            else
            {
                e.type = Event::GamepadButtonReleased;
            }
            
            e.gamepadButton = i;
            events.push(e);
        }
    }

compareAxes:
    for (uint8_t i = 0; i < JAWL_GAMEPAD_AXIS::TOTAL_AXES; i++)
    {
        if (current.stickAxes[i] == previous.stickAxes[i])
        {
            continue;
        }

        float axisPercent = (float)current.stickAxes[i] / (float)current.maxAxisValue * 100.f;
        if (std::abs(axisPercent) > JAWL_JOYSTICK_DEADZONE_PERCENT)
        {
            Event e;
            e.type = Event::GamepadAxisMoved;
            e.axisInfo.axis = i;
            e.axisInfo.value = current.stickAxes[i];

            events.push(e);
        }
    }

    if (current.triggers == previous.triggers)
    {
        return;
    }

    Event e;
    if (((current.triggers >> 8) & 0xFF) != ((previous.triggers >> 8) & 0xFF))
    {
        e.type = Event::GamepadTrigger;
        e.triggerInfo.trigger = 0;
        e.triggerInfo.value = ((current.triggers >> 8) & 0xFF);
    }

    if ((current.triggers & 0xFF) != (previous.triggers & 0xFF))
    {
        e.type = Event::GamepadTrigger;
        e.triggerInfo.trigger = 1;
        e.triggerInfo.value = (current.triggers & 0xFF);
    }

    events.push(e);
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
    previousGamepadStates.clear();
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
                previousGamepadStates.emplace_back(pad);
                printf("Gamepad Connected (Device: %d): %#x\n", i, pad.vid);
            }
        }
    }
}