# lumina-core/lumina-desktop/src-screensaver build, CMake edition. EXPERIMENTAL.
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2022 Adriaan de Groot <adridg@FreeBSD.org>
add_library(
    integrated-screensaver
    STATIC
    LLockScreen.cpp
    LScreenSaver.cpp
    SSBaseWidget.cpp
    animations/BaseAnimGroup.cpp
)
target_link_libraries(integrated-screensaver PRIVATE libLumina Qt5::Quick)
