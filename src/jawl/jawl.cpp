#include "jawl.hpp"

namespace jawl
{
    Window *init()
    {
        #ifdef _WIN32
            return new Win32_Window();
        #endif

        #ifdef __linux__
        #endif
    }
}