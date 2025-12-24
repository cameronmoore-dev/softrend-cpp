#pragma once

#include <string>
#include <cstdint>

#include "win32/win32_window.h"

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