# lumina-core build, CMake edition. EXPERIMENTAL.
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2022 Adriaan de Groot <adridg@FreeBSD.org>
cmake_minimum_required(VERSION 3.22 FATAL_ERROR)
project(lumina VERSION 1.6.2 LANGUAGES C CXX)

# Source tree selection.
#   src-qt5/  Pristine Qt5-only source for platforms that can only support Qt5.
#   src-qt6/  Qt6 source (also builds against Qt5 via version guards).
option(WITH_QT6 "Build the Qt6 source tree (src-qt6) instead of the Qt5 tree (src-qt5)" OFF)

if(WITH_QT6)
    message(STATUS "Lumina: building Qt6 source tree (src-qt6)")
    add_subdirectory(src-qt6)
else()
    message(STATUS "Lumina: building Qt5 source tree (src-qt5)")
    add_subdirectory(src-qt5)
endif()
