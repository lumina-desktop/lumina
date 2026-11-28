# lumina-core/colors build, CMake edition. EXPERIMENTAL.
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2025 jt (q5sys) <jt@obs-sec.com>

# NOTE: This directory contains legacy color scheme files
# These are pre-theme-engine files that are not used anymore
# They have been replaced by the lumina-theme-engine system
# This CMake file is provided for completeness but the files are deprecated

# Install legacy color scheme files
# These .qss.colors files were used before the theme engine was implemented
install(
    FILES
        Black.qss.colors
        Blue-Light.qss.colors
        Grey-Dark.qss.colors
        Lumina-Glass.qss.colors
        Lumina-Gold.qss.colors
        Lumina-Green.qss.colors
        Lumina-Purple.qss.colors
        Lumina-Red.qss.colors
        PCBSD10-Default.qss.colors
        Solarized-Dark.qss.colors
        Solarized-Light.qss.colors
    DESTINATION ${CMAKE_INSTALL_DATADIR}/lumina-desktop/colors
)

message(STATUS "Legacy color files installed (deprecated - use lumina-theme-engine instead)")
