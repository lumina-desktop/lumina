//===========================================
//  Lumina-DE source code
//  Copyright (c) 2026, Lumina Contributors
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  Implementation of the X11 platform abstraction layer.
//  Qt5 path uses QX11Info directly.
//  Qt6 path uses QNativeInterface::QX11Application.
//===========================================
#include "LuminaX11Bridge.h"

#include <QDebug>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
// Qt6: X11 access through native interface
#include <QGuiApplication>
#include <xcb/xcb_aux.h>
#else
// Qt5: X11 access through QX11Info
#include <QX11Info>
#endif

// ============================================================
// connection()
// ============================================================
xcb_connection_t* LuminaX11Bridge::connection()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    auto *x11App = qGuiApp->nativeInterface<QNativeInterface::QX11Application>();
    if (x11App) {
        return x11App->connection();
    }
    qWarning() << "LuminaX11Bridge::connection() - No X11 native interface available";
    return nullptr;
#else
    return QX11Info::connection();
#endif
}

// ============================================================
// display()
// ============================================================
Display* LuminaX11Bridge::display()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    auto *x11App = qGuiApp->nativeInterface<QNativeInterface::QX11Application>();
    if (x11App) {
        return x11App->display();
    }
    qWarning() << "LuminaX11Bridge::display() - No X11 native interface available";
    return nullptr;
#else
    return QX11Info::display();
#endif
}

// ============================================================
// appRootWindow()
// ============================================================
xcb_window_t LuminaX11Bridge::appRootWindow()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    // Qt6 has no direct equivalent to QX11Info::appRootWindow().
    // Retrieve it from the XCB screen structure instead.
    xcb_connection_t *conn = connection();
    if (!conn) { return 0; }
    xcb_screen_t *screen = xcb_aux_get_screen(conn, appScreen());
    if (!screen) { return 0; }
    return screen->root;
#else
    return QX11Info::appRootWindow();
#endif
}

// ============================================================
// appScreen()
// ============================================================
int LuminaX11Bridge::appScreen()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    // Qt6 has no direct equivalent to QX11Info::appScreen().
    // Use the default screen from the XCB connection setup.
    xcb_connection_t *conn = connection();
    if (!conn) { return 0; }
    // xcb_get_setup returns the setup data which includes the
    // default screen, but there is no direct API for the screen index.
    // The preferred_screen value from xcb_connect is not exposed,
    // so we default to screen 0 which is correct for single-display setups.
    // For multi-screen setups, this may need refinement.
    return 0;
#else
    return QX11Info::appScreen();
#endif
}

// ============================================================
// getTimestamp()
// ============================================================
xcb_timestamp_t LuminaX11Bridge::getTimestamp()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    // Qt6 has no direct equivalent to QX11Info::getTimestamp().
    // Use XCB_CURRENT_TIME as a fallback. This may need a more
    // sophisticated approach if timestamp-dependent operations
    // (like EWMH window activation) fail.
    // TODO: Implement proper timestamp retrieval via property change
    //       round-trip if XCB_CURRENT_TIME proves insufficient.
    return XCB_CURRENT_TIME;
#else
    return QX11Info::getTimestamp();
#endif
}

// ============================================================
// appTime()
// ============================================================
xcb_timestamp_t LuminaX11Bridge::appTime()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    // Qt6 has no direct equivalent to QX11Info::appTime().
    // Same fallback strategy as getTimestamp().
    // TODO: Implement proper timestamp retrieval if needed for
    //       RandR configuration calls.
    return XCB_CURRENT_TIME;
#else
    return QX11Info::appTime();
#endif
}
