# Theme installation, CMake edition. EXPERIMENTAL.
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2022 Adriaan de Groot <adridg@FreeBSD.org>

# This is assumed to be used from inside the icon-theme/CMakeLists,
# which in turn is part of the lumina-core build -- even though
# the source-repository layout doesn't suggest that is the case.

install_theme(
    NAME "material-design-dark"
    TARGET_SUBDIR "scalable"
    TOPLEVEL 
        "index.theme"
        "LICENSE"
    SUBDIRS
        actions
        applications
        categories
        devices
        emblems
        emotes
        international
        mimetypes
        places
        status
)
