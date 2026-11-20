# lumina-core/lumina-session build, CMake edition. EXPERIMENTAL.
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2022 Adriaan de Groot <adridg@FreeBSD.org>

add_executable(start-lumina-desktop main.cpp session.cpp)

lumina_add_manpage(start-lumina-desktop start-lumina-desktop.8 8)

target_link_libraries(
    start-lumina-desktop
    PRIVATE libLumina ${QT}::Core ${QT}::Widgets
)

install(TARGETS start-lumina-desktop)
install(PROGRAMS lumina-session.sh DESTINATION /etc/X11/Sessions RENAME lumina)

