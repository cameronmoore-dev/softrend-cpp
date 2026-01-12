#include "win32_input.h"

namespace jawl
{
    bool isKeyPressed(Key key)
    {
        return ((GetAsyncKeyState(jawlToVK(key)) & 0x8000) != 0);
    }
    
    uint16_t getKey(Key key)
    {
        return (uint16_t)jawlToVK(key);
    }
    
    void pollGamepads(RAWHID &hidData, std::queue<Event> &events)
    {
        int numXInputDevices = 0;
        for (size_t i = 0; i < _priv::g_gamepads.size(); i++)
        {
            Gamepad &gamepad = _priv::g_gamepads[i];
            Gamepad &previousState = _priv::g_previousGamepadStates[i];
    
            switch (gamepad.vid)
            {
                case GamepadVendorID::Xbox:
                {
                    XINPUT_STATE state;
                    ZeroMemory(&state, sizeof(XINPUT_STATE));
            
                    XInputGetState(numXInputDevices, &state);
                    xboxToJAWL(gamepad, state);
    
                    numXInputDevices++;
                } break;
        
                case GamepadVendorID::Dualshock4:
                {
                    dualshockToJAWL(gamepad, hidData.bRawData);
                } break;
                
                default: break;
            }
            
            compareGamepadStates(gamepad, previousState, events);
            previousState = gamepad;
        }
    }

    void xboxToJAWL(Gamepad &gamepad, XINPUT_STATE &state)
    {
        gamepad.buttons = state.Gamepad.wButtons;
        gamepad.stickAxes[GamepadAxis::LX] = state.Gamepad.sThumbLX;
        gamepad.stickAxes[GamepadAxis::LY] = state.Gamepad.sThumbLY;
        gamepad.stickAxes[GamepadAxis::RX] = state.Gamepad.sThumbRX;
        gamepad.stickAxes[GamepadAxis::RY] = state.Gamepad.sThumbRY;
        gamepad.triggers = ((state.Gamepad.bLeftTrigger << 8) | state.Gamepad.bRightTrigger);
        gamepad.maxAxisValue = INT16_MAX;
    }

    void dualshockToJAWL(Gamepad &gamepad, BYTE *rawData)
    {
        gamepad.stickAxes[GamepadAxis::LX] = rawData[1] - 128;
        gamepad.stickAxes[GamepadAxis::LY] = -(rawData[2] - 128);
        gamepad.stickAxes[GamepadAxis::RX] = rawData[3] - 128;
        gamepad.stickAxes[GamepadAxis::RY] = -(rawData[4] - 128);

        uint16_t buttonsRemap = 0;
        buttonsRemap ^= (((buttonsRemap >> GamepadButtons::DpadUp) ^ (rawData[5] >> 0)) & 0x1) << GamepadButtons::DpadUp;
        buttonsRemap ^= (((buttonsRemap >> GamepadButtons::DpadDown) ^ (rawData[5] >> 1)) & 0x1) << GamepadButtons::DpadDown;
        buttonsRemap ^= (((buttonsRemap >> GamepadButtons::DpadLeft) ^ (rawData[5] >> 2)) & 0x1) << GamepadButtons::DpadLeft;
        buttonsRemap ^= (((buttonsRemap >> GamepadButtons::DpadRight) ^ (rawData[5] >> 3)) & 0x1) << GamepadButtons::DpadRight;
        buttonsRemap ^= (((buttonsRemap >> GamepadButtons::Start) ^ (rawData[6] >> 5)) & 0x1) << GamepadButtons::Start;
        buttonsRemap ^= (((buttonsRemap >> GamepadButtons::Back) ^ (rawData[6] >> 4)) & 0x1) << GamepadButtons::Back;
        buttonsRemap ^= (((buttonsRemap >> GamepadButtons::LeftStick) ^ (rawData[6] >> 6)) & 0x1) << GamepadButtons::LeftStick;
        buttonsRemap ^= (((buttonsRemap >> GamepadButtons::RightStick) ^ (rawData[6] >> 7)) & 0x1) << GamepadButtons::RightStick;
        buttonsRemap ^= (((buttonsRemap >> GamepadButtons::LeftShoulder) ^ (rawData[6] >> 0)) & 0x1) << GamepadButtons::LeftShoulder;
        buttonsRemap ^= (((buttonsRemap >> GamepadButtons::RightShoulder) ^ (rawData[6] >> 1)) & 0x1) << GamepadButtons::RightShoulder;
        buttonsRemap ^= (((buttonsRemap >> GamepadButtons::A) ^ (rawData[5] >> 5)) & 0x1) << GamepadButtons::A;
        buttonsRemap ^= (((buttonsRemap >> GamepadButtons::B) ^ (rawData[5] >> 6)) & 0x1) << GamepadButtons::B;
        buttonsRemap ^= (((buttonsRemap >> GamepadButtons::X) ^ (rawData[5] >> 4)) & 0x1) << GamepadButtons::X;
        buttonsRemap ^= (((buttonsRemap >> GamepadButtons::Y) ^ (rawData[5] >> 7)) & 0x1) << GamepadButtons::Y;

        gamepad.buttons = buttonsRemap;
        gamepad.triggers = ((rawData[8] << 8) | rawData[9]);
        gamepad.maxAxisValue = INT8_MAX;
    }

    void storeGamepads()
    {
        UINT size = 0;
        ::GetRawInputDeviceList(nullptr, &size, sizeof(RAWINPUTDEVICELIST));

        if (!size)
        {
            printf("no devices were found!\n");
            return;
        }
        
        std::vector<RAWINPUTDEVICELIST> devices(size);
        ::GetRawInputDeviceList(devices.data(), &size, sizeof(RAWINPUTDEVICELIST));
        _priv::g_gamepads.clear();
        _priv::g_previousGamepadStates.clear();

        for (UINT i = 0; i < size; i++)
        {
            if (devices[i].dwType == RIM_TYPEHID)
            {
                RID_DEVICE_INFO info;
                ::GetRawInputDeviceInfo(devices[i].hDevice, RIDI_DEVICEINFO, nullptr, &size);
                ::GetRawInputDeviceInfo(devices[i].hDevice, RIDI_DEVICEINFO, &info, &size);

                /* Is this device a gamepad */
                if (info.hid.usUsage == 0x5)
                {
                    Gamepad pad {};
                    pad.vid = info.hid.dwVendorId;
                    _priv::g_gamepads.emplace_back(pad);
                    _priv::g_previousGamepadStates.emplace_back(pad);
                    printf("Gamepad Connected (Device: %d): %#x\n", i, pad.vid);
                }
            }
        }
    }

    uint16_t jawlToVK(Key key)
    {
        switch (key)
        {
            case Key::A:          return 'A';
            case Key::B:          return 'B';
            case Key::C:          return 'C';
            case Key::D:          return 'D';
            case Key::E:          return 'E';
            case Key::F:          return 'F';
            case Key::G:          return 'G';
            case Key::H:          return 'H';
            case Key::I:          return 'I';
            case Key::J:          return 'J';
            case Key::K:          return 'K';
            case Key::L:          return 'L';
            case Key::M:          return 'M';
            case Key::N:          return 'N';
            case Key::O:          return 'O';
            case Key::P:          return 'P';
            case Key::Q:          return 'Q';
            case Key::R:          return 'R';
            case Key::S:          return 'S';
            case Key::T:          return 'T';
            case Key::U:          return 'U';
            case Key::V:          return 'V';
            case Key::W:          return 'W';
            case Key::X:          return 'X';
            case Key::Y:          return 'Y';
            case Key::Z:          return 'Z';
            case Key::Num0:       return '0';
            case Key::Num1:       return '1';
            case Key::Num2:       return '2';
            case Key::Num3:       return '3';
            case Key::Num4:       return '4';
            case Key::Num5:       return '5';
            case Key::Num6:       return '6';
            case Key::Num7:       return '7';
            case Key::Num8:       return '8';
            case Key::Num9:       return '9';
            case Key::Escape:     return VK_ESCAPE;
            case Key::LControl:   return VK_LCONTROL;
            case Key::LShift:     return VK_LSHIFT;
            case Key::LAlt:       return VK_LMENU;
            case Key::LSystem:    return VK_LWIN;
            case Key::RControl:   return VK_RCONTROL;
            case Key::RShift:     return VK_RSHIFT;
            case Key::RAlt:       return VK_RMENU;
            case Key::RSystem:    return VK_RWIN;
            case Key::Menu:       return VK_APPS;
            case Key::LBracket:   return VK_OEM_4;
            case Key::RBracket:   return VK_OEM_6;
            case Key::Semicolon:  return VK_OEM_1;
            case Key::Comma:      return VK_OEM_COMMA;
            case Key::Period:     return VK_OEM_PERIOD;
            case Key::Apostrophe: return VK_OEM_7;
            case Key::Slash:      return VK_OEM_2;
            case Key::Backslash:  return VK_OEM_5;
            case Key::Grave:      return VK_OEM_3;
            case Key::Equal:      return VK_OEM_PLUS;
            case Key::Hyphen:     return VK_OEM_MINUS;
            case Key::Space:      return VK_SPACE;
            case Key::Enter:      return VK_RETURN;
            case Key::Backspace:  return VK_BACK;
            case Key::Tab:        return VK_TAB;
            case Key::PageUp:     return VK_PRIOR;
            case Key::PageDown:   return VK_NEXT;
            case Key::End:        return VK_END;
            case Key::Home:       return VK_HOME;
            case Key::Insert:     return VK_INSERT;
            case Key::Delete:     return VK_DELETE;
            case Key::Add:        return VK_ADD;
            case Key::Subtract:   return VK_SUBTRACT;
            case Key::Multiply:   return VK_MULTIPLY;
            case Key::Divide:     return VK_DIVIDE;
            case Key::Left:       return VK_LEFT;
            case Key::Right:      return VK_RIGHT;
            case Key::Up:         return VK_UP;
            case Key::Down:       return VK_DOWN;
            case Key::Numpad0:    return VK_NUMPAD0;
            case Key::Numpad1:    return VK_NUMPAD1;
            case Key::Numpad2:    return VK_NUMPAD2;
            case Key::Numpad3:    return VK_NUMPAD3;
            case Key::Numpad4:    return VK_NUMPAD4;
            case Key::Numpad5:    return VK_NUMPAD5;
            case Key::Numpad6:    return VK_NUMPAD6;
            case Key::Numpad7:    return VK_NUMPAD7;
            case Key::Numpad8:    return VK_NUMPAD8;
            case Key::Numpad9:    return VK_NUMPAD9;
            case Key::F1:         return VK_F1;
            case Key::F2:         return VK_F2;
            case Key::F3:         return VK_F3;
            case Key::F4:         return VK_F4;
            case Key::F5:         return VK_F5;
            case Key::F6:         return VK_F6;
            case Key::F7:         return VK_F7;
            case Key::F8:         return VK_F8;
            case Key::F9:         return VK_F9;
            case Key::F10:        return VK_F10;
            case Key::F11:        return VK_F11;
            case Key::F12:        return VK_F12;
            case Key::F13:        return VK_F13;
            case Key::F14:        return VK_F14;
            case Key::F15:        return VK_F15;
            case Key::Pause:      return VK_PAUSE;
            default:                   return 0;
        }
    }
}