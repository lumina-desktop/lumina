# lumina-core-utils/lcc/window-manager build, CMake edition. EXPERIMENTAL.
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2022 Adriaan de Groot <adridg@FreeBSD.org>

# LCC Window-manager Module

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
add_executable(lcc-window-manager
    ${SOURCES}
    ${HEADERS}
    ${UI_HEADERS}
)

# Include directories
target_include_directories(lcc-window-manager PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}  # For UI headers
)

# Link libraries
target_link_libraries(lcc-window-manager PRIVATE
    libLumina
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
)

# Install executable
install(TARGETS lcc-window-manager DESTINATION bin)

# Install desktop file
install(
    FILES lcc-window-manager.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)

message(STATUS "LCC window-manager module configured")
