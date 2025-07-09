# lumina-core/lumina-pingcursor build, CMake edition. EXPERIMENTAL.
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2022 Adriaan de Groot <adridg@FreeBSD.org>

add_executable(lumina-pingcursor main.cpp)
target_link_libraries(lumina-pingcursor PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets)
install(TARGETS lumina-pingcursor)
