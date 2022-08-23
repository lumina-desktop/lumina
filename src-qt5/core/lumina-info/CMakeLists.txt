# lumina-core/lumina-open build, CMake edition. EXPERIMENTAL.
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2022 Adriaan de Groot <adridg@FreeBSD.org>

add_executable(lumina-info main.cpp MainUI.cpp)
target_link_libraries(
    lumina-info
    PRIVATE libLumina Qt5::Core Qt5::Gui Qt5::Widgets Qt5::X11Extras
)
install(TARGETS lumina-info)

# TODO: qrc, translations, manpage
