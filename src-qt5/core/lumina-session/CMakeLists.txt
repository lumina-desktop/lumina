# lumina-core/lumina-session build, CMake edition. EXPERIMENTAL.
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2022 Adriaan de Groot <adridg@FreeBSD.org>

add_executable(start-lumina-desktop main.cpp session.cpp)
target_link_libraries(
    start-lumina-desktop
    PRIVATE libLumina Qt5::Core Qt5::Widgets Qt5::X11Extras
)
install(TARGETS start-lumina-desktop)
