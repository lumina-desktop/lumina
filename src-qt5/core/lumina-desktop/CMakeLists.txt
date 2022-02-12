# lumina-core/lumina-desktop build, CMake edition. EXPERIMENTAL.
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2022 Adriaan de Groot <adridg@FreeBSD.org>

# The plugins, as well as sources in this directory, expect to
# include from this directory, from src-screensaver/, but **also* from
# the top-level (outside of lumina-core entirely!) src-cpp.
set(TOPLEVEL_SRC_CPP ${CMAKE_SOURCE_DIR}/../src-cpp/)
include_directories(
    lumina-desktop
    PRIVATE
    ${TOPLEVEL_SRC_CPP}
    ${CMAKE_CURRENT_LIST_DIR}
    src-screensaver
)

add_subdirectory(desktop-plugins)
add_subdirectory(panel-plugins)
add_subdirectory(src-screensaver)

add_executable(
    lumina-desktop
    main.cpp
    WMProcess.cpp
    LXcbEventFilter.cpp
    LSession.cpp
    LDesktop.cpp
    LDesktopBackground.cpp
    LDesktopPluginSpace.cpp
    LPanel.cpp
    LWinInfo.cpp
    AppMenu.cpp
    SettingsMenu.cpp
    SystemWindow.cpp
    BootSplash.cpp
    # Header-only classes
    JsonMenu.h
)
# FIXME: because we're CMake-ifying only core,
# the src-cpp bits lie "outside" the regular source tree.
# Make that a library at some point.
target_sources(
    lumina-desktop
    PUBLIC
        ${TOPLEVEL_SRC_CPP}/plugins-base.cpp
        ${TOPLEVEL_SRC_CPP}/plugins-desktop.cpp
        ${TOPLEVEL_SRC_CPP}/plugins-screensaver.cpp
)

# Picks up many Qt libraries from libLumina's PUBLIC settings
target_link_libraries(
    lumina-desktop
    PRIVATE
        libLumina
        Qt5::Gui
        Qt5::Network
        Qt5::Quick
        Qt5::Qml
        XCB::XCB
        XCB::COMPOSITE
        XCB::DAMAGE
        XCB::DPMS
        XCB::EWMH
        XCB::ICCCM
        XCB::IMAGE
)
# The panel-plugins have their own static library
target_link_libraries(
    lumina-desktop
    PRIVATE integrated-desktop integrated-panel integrated-screensaver
)
install(TARGETS lumina-desktop)

# Additional files
install(
    FILES
        audiofiles/Login.ogg
        audiofiles/Logout.ogg
        audiofiles/low-battery.ogg
        fluxboxconf/fluxbox-init-rc
        fluxboxconf/fluxbox-keys
        fluxboxconf/theme.cfg
    DESTINATION ${CMAKE_INSTALL_DATADIR}/lumina-desktop/
)
install(FILES Lumina-DE.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/xsessions/)
install(
    FILES Lumina-DE.svg
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps/
)
