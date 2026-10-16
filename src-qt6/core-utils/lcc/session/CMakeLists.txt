# lumina-core-utils/lcc/session build, CMake edition. EXPERIMENTAL.
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2022 Adriaan de Groot <adridg@FreeBSD.org>

# LCC Session Module

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
add_executable(lcc-session
    ${SOURCES}
    ${HEADERS}
    ${UI_HEADERS}
)

# Include directories
target_include_directories(lcc-session PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}  # For UI headers
)

# Link libraries
target_link_libraries(lcc-session PRIVATE
    libLumina
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
)

# Install executable
install(TARGETS lcc-session DESTINATION bin)

# Install desktop file
install(
    FILES lcc-session.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)

message(STATUS "LCC session module configured")
