#include <windows.h>
#include <xinput.h>

#include "../input.h"

namespace jawl
{
    bool isKeyPressed(Key key);
    uint16_t getKey(Key key);
    uint16_t jawlToVK(Key key);

    void storeGamepads();
    void pollGamepads(RAWHID &hidData, std::queue<Event> &events);

    void xboxToJAWL(Gamepad &gamepad, XINPUT_STATE &state);
    void dualshockToJAWL(Gamepad &gamepad, BYTE *rawData);
}