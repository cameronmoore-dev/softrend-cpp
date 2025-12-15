#include <windows.h>
#include <stdint.h>
#include <xinput.h>

#include <vector>

#include "Events.h"

#define JAWL_LEFTSTICK_DEADZONE_PERCENT  20
#define JAWL_RIGHTSTICK_DEADZONE_PERCENT 25
#define JAWL_TRIGGER_DEADZONE 30



class Input
{
public:
    struct JAWL_Gamepad
    {
        uint16_t vid;
        uint16_t buttons;
        int16_t stickAxes[4];
        uint16_t triggers;
        uint16_t maxAxisValue = INT16_MAX;
    };

    enum JAWL_GAMEPAD_BUTTONS
    {
        DPAD_UP = 0,
        DPAD_DOWN,
        DPAD_LEFT,
        DPAD_RIGHT,
        START,
        BACK,
        LEFT_STICK,
        RIGHT_STICK,
        LEFT_SHOULDER,
        RIGHT_SHOULDER,
        A = 12,
        B,
        X,
        Y,
        TOTAL_BUTTONS
    };

    enum JAWL_GAMEPAD_AXIS
    {
        LX = 0,
        LY,
        RX,
        RY,
        TOTAL_AXES
    };

    static bool isGamepadConnected(uint8_t cid);
    static bool isGamepadButtonPressed(uint8_t cid, JAWL_GAMEPAD_BUTTONS button);
    static float getControllerAxisValue(uint8_t cid, JAWL_GAMEPAD_AXIS axis);

    void pollGamepads(RAWINPUT *raw, Event &event);
    void pollHIDRawBuffer();

    void storeGameControllers();

private:
    inline static std::vector<JAWL_Gamepad> gamepads;

private:
    void xboxToJAWL(JAWL_Gamepad &gamepad, XINPUT_STATE &state);
    void dualshockToJAWL(JAWL_Gamepad &gamepad, BYTE *rawData);
};