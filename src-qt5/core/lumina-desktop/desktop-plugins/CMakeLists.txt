# lumina-core/lumina-desktop/desktop-plugins build, CMake edition. EXPERIMENTAL.
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2022 Adriaan de Groot <adridg@FreeBSD.org>
add_library(
    integrated-desktop
    STATIC
    LDPlugin.cpp
    applauncher/AppLauncherPlugin.cpp
    applauncher/OutlineToolButton.h
    audioplayer/PlayerWidget.cpp
    desktopview/DesktopViewPlugin.cpp
    notepad/NotepadPlugin.cpp
    rssreader/RSSFeedPlugin.cpp
    rssreader/RSSObjects.cpp
    systemmonitor/MonitorWidget.cpp
    # Header-only plugins
    calendar/CalendarPlugin.h
)
target_link_libraries(integrated-desktop PRIVATE libLumina Qt5::Quick)
