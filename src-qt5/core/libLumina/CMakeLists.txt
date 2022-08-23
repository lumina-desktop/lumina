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
    LuminaXDG.cpp
    ResizeMenu.cpp
    XDGMime.cpp
    # Header-only classes
    ExternalProcess.h
)
if(CMAKE_SYSTEM MATCHES "FreeBSD")
    target_sources(libLumina PUBLIC LuminaOS-FreeBSD.cpp)
endif()

target_include_directories(libLumina PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(
    libLumina
    PUBLIC
        Qt5::Concurrent
        Qt5::Core
        Qt5::MultimediaWidgets
        Qt5::Svg
        Qt5::Widgets
        Qt5::X11Extras
        XCB::XCB
        XCB::AUX
        XCB::DAMAGE
        XCB::SCREENSAVER
        X11::Xdamage
)
