#include "os/window.h"
#include "os/_time.h"

#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <X11/Xlib-xcb.h>
#include <X11/extensions/Xfixes.h>
#include <xcb/xcb.h>
#include <libevdev-1.0/libevdev/libevdev.h>
#include <libudev.h>

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>
#include <errno.h>
#include <time.h>

#define MOUSE_BUTTON_LEFT   1
#define MOUSE_BUTTON_MIDDLE 2
#define MOUSE_BUTTON_RIGHT  3
#define SCROLL_WHEEL_UP     4
#define SCROLL_WHEEL_DOWN   5
#define XBUTTON0            8
#define XBUTTON1            9

typedef struct platform_context
{
    Display *display;
    xcb_connection_t *connection;
    xcb_window_t window;
    xcb_screen_t *screen;
    xcb_atom_t wm_deleteWindow;
    struct udev *udevice;
    struct udev_monitor *umonitor;

    xcb_gcontext_t gid;
} platform_context;

void window_on_resize(window_ *window, xcb_configure_notify_event_t *cfg);
xcb_atom_t _xcb_intern_atom(xcb_connection_t *connection, const char *atom_name);

void set_position(platform_context *platform, u32 x, u32 y)
{
    u32 mask = XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y;
    s32 values[] = { x, y };
    xcb_configure_window(platform->connection, platform->window, mask, values);
    xcb_flush(platform->connection);
}

void update_cursor(window_ *window)
{
    u32 cx = window->width / 2;
    u32 cy = window->height / 2;
    if (window->input.mx == cx && 
        window->input.my == cy)
    {
        return;
    }

    xcb_warp_pointer(
        window->platform->connection, 
        XCB_NONE, 
        window->platform->window, 
        0, 0, 0, 0, 
        cx, cy
    );

    // xcb_grab_pointer(
    //     window->platform->connection, 
    //     1, 
    //     window->platform->window, 
    //     XCB_EVENT_MASK_BUTTON_PRESS | 
    //     XCB_EVENT_MASK_BUTTON_RELEASE | 
    //     XCB_EVENT_MASK_POINTER_MOTION, 
    //     XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, 
    //     window->platform->window, 
    //     XCB_NONE, XCB_CURRENT_TIME
    // );

    xcb_flush(window->platform->connection);
}

window_ *window_create(const char *title, u32 width, u32 height)
{
    window_ *window = (window_ *)malloc(sizeof(window_));
    window->platform = (platform_context *)malloc(sizeof(platform_context));
    window->frontbuffer = (u32 *)malloc(width * height * sizeof(u32));
    window->title = title;
    window->width = width;
    window->height = height;
    window->backbuffer.data = (u32 *)malloc(width * height * sizeof(u32));
    window->backbuffer.width = width;
    window->backbuffer.height = height;
    window->isOpen = true;
    window->input = (input_){};
    window->frame_start = (timepoint_){};
    
    window->platform->display = XOpenDisplay(NULL);
    window->platform->connection = XGetXCBConnection(window->platform->display);
    
    XAutoRepeatOff(window->platform->display);
    XSetEventQueueOwner(window->platform->display, XCBOwnsEventQueue);

    const struct xcb_setup_t *setup = xcb_get_setup(window->platform->connection);
    xcb_screen_iterator_t it = xcb_setup_roots_iterator(setup);
    for (s32 i = 0; i > 0; i--)
    {
        xcb_screen_next(&it);
    }

    window->platform->screen = it.data;
    window->platform->window = xcb_generate_id(window->platform->connection);

    u32 event_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    u32 event_values = 
        XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE | 
        XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE | 
        XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_POINTER_MOTION | 
        XCB_EVENT_MASK_FOCUS_CHANGE | XCB_EVENT_MASK_STRUCTURE_NOTIFY;

    u32 value_list[] = 
    {
        window->platform->screen->black_pixel, 
        event_values 
    };

    xcb_create_window(
        window->platform->connection,
        XCB_COPY_FROM_PARENT,
        window->platform->window,
        window->platform->screen->root,
        0, 0,
        width, height,
        0,
        XCB_WINDOW_CLASS_INPUT_OUTPUT,
        window->platform->screen->root_visual,
        event_mask,
        value_list
    );

    xcb_change_property(
        window->platform->connection, 
        XCB_PROP_MODE_REPLACE, 
        window->platform->window, 
        XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 
        8, strlen(title), title
    );

    window->platform->wm_deleteWindow = _xcb_intern_atom(window->platform->connection, "WM_DELETE_WINDOW");
    xcb_atom_t proto = _xcb_intern_atom(window->platform->connection, "WM_PROTOCOLS");
    xcb_change_property(
        window->platform->connection,
        XCB_PROP_MODE_REPLACE,
        window->platform->window, proto,
        4, 32, 1,
        &window->platform->wm_deleteWindow
    );
    
    window->platform->udevice = udev_new();
    window->platform->umonitor = udev_monitor_new_from_netlink(window->platform->udevice, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(window->platform->umonitor, "input", NULL);
    udev_monitor_enable_receiving(window->platform->umonitor);
    
    window->platform->gid = xcb_generate_id(window->platform->connection);
    xcb_create_gc(window->platform->connection, window->platform->gid, window->platform->window, 0, NULL);

    XFixesHideCursor(window->platform->display, window->platform->window);

    xcb_map_window(window->platform->connection, window->platform->window);
    xcb_flush(window->platform->connection);

    return window;
}

void window_swap_buffers(window_ *window)
{
    update_cursor(window);

    if (window->frontbuffer && 
        window->backbuffer.data)
    {
        u32 buffer_size_bytes = (window->backbuffer.width * window->backbuffer.height) * sizeof(u32);

        memcpy(window->frontbuffer, window->backbuffer.data, buffer_size_bytes);

        xcb_put_image(
            window->platform->connection, XCB_IMAGE_FORMAT_Z_PIXMAP, 
            window->platform->window, window->platform->gid, 
            window->width, window->height, 
            0, 0, 
            0, window->platform->screen->root_depth, 
            buffer_size_bytes,
            (const u8 *)window->frontbuffer
        );

        xcb_flush(window->platform->connection);
    }
}

void window_sleep(window_ *window)
{
    duration_  target_frametime = set_duration(1.0 / 60.f);
    timepoint_ next_frame_timepoint = get_future_point(&window->frame_start, &target_frametime);
    timepoint_ now = get_time();

    duration_ diff = timepoint_diff(&next_frame_timepoint, &now);

    struct timespec ts = {};
    ts.tv_nsec = diff.ns;
    nanosleep(&ts, NULL);
    
    window->frame_start = get_time();
}

void window_cleanup(window_ *window)
{
    XAutoRepeatOn(window->platform->display);
    // xcb_ungrab_pointer(window->platform->connection, XCB_CURRENT_TIME);
    xcb_destroy_window(window->platform->connection, window->platform->window);
    xcb_free_gc(window->platform->connection, window->platform->gid);
    udev_unref(window->platform->udevice);
    udev_monitor_unref(window->platform->umonitor);

    free(window->backbuffer.data);
    free(window->frontbuffer);
    free(window->platform);
    free(window);
}

void window_pump_messages(window_ *window)
{
    xcb_generic_event_t *event;
    while ((event = xcb_poll_for_event(window->platform->connection)))
    {
        switch (event->response_type & ~0x80)
        {
            case XCB_CLIENT_MESSAGE:
            {
                xcb_client_message_event_t *client = (xcb_client_message_event_t *)event;
                if (client->data.data32[0] == window->platform->wm_deleteWindow)
                {
                    window_close(window);
                }
                break;
            } 

            case XCB_CONFIGURE_NOTIFY:
            {
                xcb_configure_notify_event_t *cfg = (xcb_configure_notify_event_t *)event;
                window_on_resize(window, cfg);
                break;  
            } 

            case XCB_FOCUS_IN:
            {
                XFixesHideCursor(window->platform->display, window->platform->window);

                u32 cx = window->width / 2;
                u32 cy = window->height / 2;
                xcb_warp_pointer(
                    window->platform->connection, 
                    XCB_NONE, 
                    window->platform->window, 
                    0, 0, 0, 0, 
                    cx, cy
                );
            } break;

            case XCB_FOCUS_OUT:
            {
                XFixesShowCursor(window->platform->display, window->platform->window);
            } break;

            case XCB_MOTION_NOTIFY:
            {
                xcb_motion_notify_event_t *mouse = (xcb_motion_notify_event_t *)event;

                u32 cx = window->width / 2;
                u32 cy = window->height / 2;

                window->input.mouse_dx = mouse->event_x - cx;
                window->input.mouse_dy = mouse->event_y - cy;
                window->input.mx = mouse->event_x;
                window->input.my = mouse->event_y;
                
            } break;

            case XCB_KEY_PRESS:
            {
                xcb_key_press_event_t *kp = (xcb_key_press_event_t *)event;
                xcb_keycode_t code = kp->detail;
                KeySym key = XkbKeycodeToKeysym(window->platform->display, (KeyCode)code, 0, 1);

                _set_key_pressed(&window->input, key);
            } break;

            case XCB_KEY_RELEASE:
            {
                xcb_key_press_event_t *kp = (xcb_key_press_event_t *)event;
                xcb_keycode_t code = kp->detail;
                KeySym key = XkbKeycodeToKeysym(window->platform->display, (KeyCode)code, 0, 1);

                _set_key_released(&window->input, key);
            } break;

            default: break;
        }

        free(event);
    }
}



bool window_is_open(window_ *window)
{
    return window->isOpen;
}

void window_close(window_ *window)
{
    window->isOpen = false;
}

void window_on_resize(window_ *window, xcb_configure_notify_event_t *cfg)
{
    window->width  = cfg->width;
    window->height = cfg->height;

    // TODO: Use realloc()
    if (window->frontbuffer)       free(window->frontbuffer);
    if (window->backbuffer.data)   free(window->backbuffer.data);
    window->frontbuffer = (u32 *)malloc(cfg->width * cfg->height * sizeof(u32));
    window->backbuffer.data = (u32 *)malloc(cfg->width * cfg->height * sizeof(u32));
    window->backbuffer.width  = cfg->width;
    window->backbuffer.height = cfg->height;
}

xcb_atom_t _xcb_intern_atom(xcb_connection_t *connection, const char *atom_name)
{
    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(connection, 0, strlen(atom_name), atom_name);
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(connection, cookie, NULL);
    xcb_atom_t atom = reply->atom;

    free(reply);
    return atom;
}