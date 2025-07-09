# lumina-core-utils/lcc/panels build, CMake edition. EXPERIMENTAL.
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2022 Adriaan de Groot <adridg@FreeBSD.org>

# LCC Panels Module

# Find required Qt5 modules
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)

# Source files
set(SOURCES
    main.cpp
    MainWindow.cpp
)

set(HEADERS
    MainWindow.h
)

set(FORMS
    MainWindow.ui
)

# Process UI forms
qt5_wrap_ui(UI_HEADERS ${FORMS})

# Create the executable
add_executable(lcc-panels
    ${SOURCES}
    ${HEADERS}
    ${UI_HEADERS}
)

# Include directories
target_include_directories(lcc-panels PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}  # For UI headers
)

# Link libraries
target_link_libraries(lcc-panels PRIVATE
    libLumina
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
)

# Install executable
install(TARGETS lcc-panels DESTINATION bin)

# Install desktop file
install(
    FILES lcc-panels.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)

message(STATUS "LCC panels module configured")
