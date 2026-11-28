//===========================================
//  Lumina-DE source code
//  Copyright (c) 2026, Lumina Contributors
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  Abstraction layer for X11 platform access.
//  Centralizes all QX11Info usage so that the
//  Qt5-to-Qt6 conditional logic exists in one
//  place instead of 200+ individual call sites.
//===========================================
#ifndef LUMINA_X11_BRIDGE_H
#define LUMINA_X11_BRIDGE_H

#include <QtGlobal>
#include <xcb/xcb.h>

// X11/Xlib forward declaration
typedef struct _XDisplay Display;

class LuminaX11Bridge {
public:
    // Returns the XCB connection for the running X11 session.
    // This is the most frequently used accessor (~200+ call sites).
    static xcb_connection_t* connection();

    // Returns the Xlib Display pointer for legacy Xlib calls.
    // Used by DPMS, XDamage, Xcursor, and desktop switcher code.
    static Display* display();

    // Returns the root window for the default screen.
    // Used extensively in EWMH, ICCCM, and RandR operations.
    static xcb_window_t appRootWindow();

    // Returns the default screen number.
    // Used for EWMH screen-indexed operations and system tray setup.
    static int appScreen();

    // Returns a server timestamp suitable for EWMH requests.
    // Used in window activation and close requests.
    static xcb_timestamp_t getTimestamp();

    // Returns the application time from the X server.
    // Used in RandR configuration calls.
    static xcb_timestamp_t appTime();

private:
    LuminaX11Bridge() = delete; // Static-only class, no instantiation
};

#endif // LUMINA_X11_BRIDGE_H
