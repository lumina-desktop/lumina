# lumina-core/lumina-desktop/panel-plugins build, CMake edition. EXPERIMENTAL.
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2022 Adriaan de Groot <adridg@FreeBSD.org>
add_library(
    integrated-panel
    STATIC
    applauncher/AppLaunchButton.cpp
    appmenu/LAppMenuPlugin.cpp
    audioplayer/LPAudioPlayer.cpp
    audioplayer/PPlayerWidget.cpp
    battery/LBattery.cpp
    clock/LClock.cpp
    desktopbar/LDeskBar.cpp
    desktopswitcher/LDesktopSwitcher.cpp
    showdesktop/LHomeButton.cpp
    systemdashboard/LSysDashboard.cpp
    systemdashboard/SysMenuQuick.cpp
    systemtray/LSysTray.cpp
    systemtray/TrayIcon.cpp
    taskmanager/LTaskManagerPlugin.cpp
    taskmanager/LTaskButton.cpp
    userbutton/UserWidget.cpp
    userbutton/UserItemWidget.cpp
    systemstart/LStartButton.cpp
    systemstart/StartMenu.cpp
    systemstart/ItemWidget.cpp
    jsonmenu/PPJsonMenu.cpp
)
target_link_libraries(integrated-panel PRIVATE libLumina Qt5::Quick)
