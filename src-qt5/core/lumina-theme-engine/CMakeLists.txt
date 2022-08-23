# lumina-core/lumina-theme-engine build, CMake edition. EXPERIMENTAL.
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2022 Adriaan de Groot <adridg@FreeBSD.org>

add_subdirectory(src/lthemeengine)
add_subdirectory(src/lthemeengine-qtplugin)
add_subdirectory(src/lthemeengine-sstest)
add_subdirectory(src/lthemeengine-style)

install(
    DIRECTORY colors
    DESTINATION ${CMAKE_INSTALL_DATADIR}/lthemeengine/colors
    FILES_MATCHING
    PATTERN *.conf
)
install(
    DIRECTORY qss
    DESTINATION ${CMAKE_INSTALL_DATADIR}/lthemeengine/qss
    FILES_MATCHING
    PATTERN *.qss
)
install(
    DIRECTORY desktop_qss
    DESTINATION ${CMAKE_INSTALL_DATADIR}/lthemeengine/desktop_qss
    FILES_MATCHING
    PATTERN *.qss
)
