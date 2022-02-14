# lumina-core/lumina-open build, CMake edition. EXPERIMENTAL.
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2022 Adriaan de Groot <adridg@FreeBSD.org>

add_executable(lumina-open main.cpp LFileDialog.cpp)
target_link_libraries(
    lumina-open
    PRIVATE libLumina Qt5::Core Qt5::Gui Qt5::Widgets Qt5::X11Extras
)
install(TARGETS lumina-open)

# TODO: qrc, translations, manpage
