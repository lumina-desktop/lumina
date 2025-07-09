# lumina-core/lumina-session build, CMake edition. EXPERIMENTAL.
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2022 Adriaan de Groot <adridg@FreeBSD.org>

add_executable(start-lumina-desktop main.cpp session.cpp)

add_custom_command(     
        TARGET start-lumina-desktop
        POST_BUILD
        COMMAND gzip -c ${CMAKE_CURRENT_SOURCE_DIR}/start-lumina-desktop.8 > ${CMAKE_CURRENT_BINARY_DIR}/start-lumina-desktop.8.gz
        BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/start-lumina-desktop.8.gz
)

target_link_libraries(
    start-lumina-desktop
    PRIVATE libLumina Qt5::Core Qt5::Widgets Qt5::X11Extras
)

install(TARGETS start-lumina-desktop)
install(PROGRAMS lumina-session.sh DESTINATION /etc/X11/Sessions RENAME lumina)

install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/start-lumina-desktop.8.gz
        DESTINATION ${CMAKE_INSTALL_DATADIR}/man/man8
)
