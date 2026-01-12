#include "input.h"

namespace jawl
{
    bool isGamepadConnected(uint8_t slot)
    {
        return slot < _priv::g_gamepads.size();
    }

    bool isGamepadButtonPressed(uint8_t slot, GamepadButtons button)
    {
        if (slot >= _priv::g_gamepads.size())
        {
            return false;
        }

        return ((_priv::g_gamepads[slot].buttons >> button) & 0x1);
    }

    float controllerAxisValue(uint8_t slot, GamepadAxis axis)
    {
        if (slot >= _priv::g_gamepads.size())
        {
            return -1.f;
        }
    
        Gamepad &gamepad = _priv::g_gamepads[slot];
        float axisValue = (float)gamepad.stickAxes[(int)axis];
        float whole = (float)gamepad.maxAxisValue;
        float axisPercent = std::floor(axisValue / whole * 100.f);
    
        if (std::abs(axisPercent) > GamepadDeadzone::JoystickPercent)
        {
            /* Normalise value between -1.0 and 1.0 */
            return (axisValue / whole);
        }
        
        return 0.f;
    }

    void compareGamepadStates(Gamepad &current, Gamepad &previous, std::queue<Event> &events)
    {
        if (current.buttons == previous.buttons)
        {
            goto compareAxes;
        }

        for (uint8_t i = 0; i < GamepadButtons::ButtonCount; i++)
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
        for (uint8_t i = 0; i < GamepadAxis::AxesCount; i++)
        {
            if (current.stickAxes[i] == previous.stickAxes[i])
            {
                continue;
            }

            float axisPercent = (float)current.stickAxes[i] / (float)current.maxAxisValue * 100.f;
            if (std::abs(axisPercent) > GamepadDeadzone::JoystickPercent)
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
}