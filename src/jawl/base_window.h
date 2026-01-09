#pragma once

#include <cstdint>
#include <array>
#include <vector>
#include <queue>
#include <string>
#include <chrono>

#include "input.h"
#include "event.h"

namespace jawl
{
    enum GLContextProfile
    {
        #ifdef _WIN32
            Core            = 0x1,
            Compatibility   = 0x2
        #endif
    };

    enum Style
    {
        #ifdef _WIN32
            Titlebar = 0x00C00000L, /* WS_CAPTION */
            Resizable = 0x00040000L, /* WS_SIZEBOX */
            Maximised = 0x01000000L, /* WS_MAXIMIZE */
            Maximisable = 0x00C00000L | 0x00080000L | 0x00010000L, /* WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX */
            Minimisable = 0x00C00000L | 0x00080000L | 0x00020000L, /* WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX */
            Default = 0x00000000L | 0x00C00000L | 0x00080000L | 0x00040000L | 0x00010000L | 0x00020000L /* WS_OVERLAPPEDWINDOW */
        #endif
    };

    enum class RendererType
    {
        Software = 0,
        Hardware
    };

    enum CursorFlags
    {
        Hidden  = 0x1,
        Locked  = 0x2,
        Clamped = 0x4
    };

    enum Hints
    {
        ColourBits = 0,
        DepthBits,
        StencilBits,
        Samples,
        sRGBCapable,
        DoubleBuffer,
        ContextVerMajor,
        ContextVerMinor,
        ContextProfile,
        HintsCount
    };

    struct Backbuffer;

    class Window
    {
    public:
        Window();
        virtual ~Window() {}

        void setHint(Hints index, uint8_t value);
        void close();
        bool pollEvents(Event &event);
        void cursorEnableFlags(const uint8_t flags);
        void cursorDisableFlags(const uint8_t flags);
    
        const Backbuffer *backbuffer() { return m_backbuffer; } const
        
        virtual Window *create(const std::string &title, const uint16_t width, const uint16_t height, const uint32_t style, const RendererType rtype) = 0;
        virtual void cleanup() = 0;
        virtual void swapBuffers() = 0;
        virtual void swapInterval(const uint8_t interval) = 0;
        virtual void setTitle(const std::string &title) = 0;
        virtual void setFullscreen(const bool fullscreen) = 0;
        virtual void getPos(int &outX, int &outY) = 0;
        virtual void getSize(int &outWidth, int & outHeight) = 0;
        virtual void setPos(const int x, const int y) = 0;
        virtual void setSize(const int width, const int height) = 0;
        
        virtual bool isFocused() = 0;
        bool isOpen() { return m_isOpen; }
    
    protected:
        uint32_t *m_frontbuffer = nullptr;
        Backbuffer *m_backbuffer = nullptr;

        std::queue<Event> m_events;
        std::array<Hints, HintsCount> m_hints;
        
        RendererType m_rtype = RendererType::Software;
        std::string m_title;
        uint32_t m_style = 0;
        uint16_t m_width = 0;
        uint16_t m_height = 0;
        uint8_t m_cursorState = 0;
        bool m_isOpen = false;
    
    protected:
        virtual void swSetup() = 0;
        virtual void glSetup() = 0;
        virtual void registerInputDevices() = 0;
        virtual void updateCursor() = 0;
    };
}

namespace
{
    double getTimeSeconds()
    {
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> time = now.time_since_epoch();
        return time.count();
    }
}