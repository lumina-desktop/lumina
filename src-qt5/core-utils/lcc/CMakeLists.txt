# lumina-core-utils/lcc build, CMake edition. EXPERIMENTAL.
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2022 Adriaan de Groot <adridg@FreeBSD.org>

# LCC (Lumina Control Center) - Main configuration interface for Lumina Desktop
# This is a collection of configuration modules for various desktop settings

# Add all LCC subdirectory modules
add_subdirectory(main)
add_subdirectory(wallpaper)
add_subdirectory(autostart)
add_subdirectory(defaultapps)
add_subdirectory(keyboard)
add_subdirectory(window-manager)
add_subdirectory(desktop)
add_subdirectory(panels)
add_subdirectory(menu)
add_subdirectory(locale)
add_subdirectory(session)
add_subdirectory(compositor)
add_subdirectory(sound-theme)
add_subdirectory(mouse)
add_subdirectory(bluetooth)

message(STATUS "LCC (Lumina Control Center) modules configured")
