# lumina-core-utils/lcc/main build, CMake edition. EXPERIMENTAL.
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2022 Adriaan de Groot <adridg@FreeBSD.org>

# Main LCC (Lumina Control Center) executable

# Find required Qt5 modules
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets X11Extras)

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
add_executable(lcc
    ${SOURCES}
    ${HEADERS}
    ${UI_HEADERS}
)

# Include directories
target_include_directories(lcc PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}  # For UI headers
)

# Link libraries
target_link_libraries(lcc PRIVATE
    libLumina
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::X11Extras
)

# Install executable
install(TARGETS lcc DESTINATION bin)

# Install desktop file
install(
    FILES lcc.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)

# TODO: Add man page installation when lcc.1 is available
# TODO: Add translation support when i18n files are available

message(STATUS "LCC main executable configured")
