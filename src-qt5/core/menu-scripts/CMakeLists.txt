# lumina-core/menu-scripts build, CMake edition. EXPERIMENTAL.
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2025 jt (q5sys) <jt@obs-sec.com>

# Menu scripts for Lumina Desktop
# These scripts are used for generating dynamic menus in the desktop environment

# Install menu script files
install(
    FILES
        README.md
        ls.json.sh
    DESTINATION ${CMAKE_INSTALL_DATADIR}/lumina-desktop/menu-scripts
)

# Make the shell script executable
install(
    FILES ls.json.sh
    DESTINATION ${CMAKE_INSTALL_DATADIR}/lumina-desktop/menu-scripts
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ GROUP_EXECUTE
                WORLD_READ WORLD_EXECUTE
)

message(STATUS "Menu scripts installed to ${CMAKE_INSTALL_DATADIR}/lumina-desktop/menu-scripts")
