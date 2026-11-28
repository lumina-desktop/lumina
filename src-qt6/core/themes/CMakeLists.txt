# lumina-core/themes build, CMake edition. EXPERIMENTAL.
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2025 jt (q5sys) <jt@obs-sec.com>

# Theme templates for Lumina Desktop
# These QSS template files are used by the theme engine to generate themes

# Install theme template files
install(
    FILES
        DarkGlass.qss.template
        Glass.qss.template
        Lumina-default.qss.template
        None.qss.template
    DESTINATION ${CMAKE_INSTALL_DATADIR}/lumina-desktop/themes
)

message(STATUS "Theme templates installed to ${CMAKE_INSTALL_DATADIR}/lumina-desktop/themes")
