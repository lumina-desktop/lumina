# lumina-core/libLumina build, CMake edition. EXPERIMENTAL.
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2022 Adriaan de Groot <adridg@FreeBSD.org>
add_library(
    libLumina
    STATIC
    DesktopSettings.cpp
    LDesktopUtils.cpp
    LFileInfo.cpp
    LIconCache.cpp
    LInputDevice.cpp
    LUtils.cpp
    LVideoLabel.cpp
    LVideoSurface.cpp
    LVideoWidget.cpp
    LuminaRandR-X11.cpp
    LuminaSingleApplication.cpp
    LuminaThemes.cpp
    LuminaX11.cpp
    LuminaX11Bridge.cpp
    LuminaXDG.cpp
    ResizeMenu.cpp
    XDGMime.cpp
    # Header-only classes
    ExternalProcess.h
)

if (OS MATCHES "Unknown")
    target_sources(libLumina PUBLIC LuminaOS-template.cpp)
else()
    target_sources(libLumina PUBLIC LuminaOS-${OS}.cpp)
endif()

target_include_directories(libLumina PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(
    libLumina
    PUBLIC
        ${QT}::Concurrent
        ${QT}::Core
        ${QT}::MultimediaWidgets
        ${QT}::Svg
        ${QT}::Widgets
        XCB::XCB
        XCB::AUX
        XCB::DAMAGE
        XCB::EWMH
        XCB::ICCCM
        XCB::DPMS
        XCB::IMAGE
        XCB::COMPOSITE
        XCB::SCREENSAVER
        X11::Xdamage
)
# Qt5::X11Extras is only needed for Qt5; Qt6 uses QNativeInterface from Qt6::Gui
if(LUMINA_HAS_X11EXTRAS)
    target_link_libraries(libLumina PUBLIC Qt5::X11Extras)
endif()
# Qt6::Core5Compat provides QTextCodec, QRegExp, and other removed Qt5 classes
if(WITH_QT6)
    target_link_libraries(libLumina PUBLIC Qt6::Core5Compat)
endif()
