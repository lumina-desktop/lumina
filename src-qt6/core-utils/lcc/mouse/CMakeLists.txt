# lumina-core-utils/lcc/mouse build
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2025 jt (q5sys) <jt@obs-sec.com>

# LCC Mouse Module
# UI forms and moc are handled by CMAKE_AUTOUIC / CMAKE_AUTOMOC (set in src-qt6/CMakeLists.txt)
# This module calls Xlib and XInput directly, so it links X11 and Xi.

add_executable(lcc-mouse
	main.cpp
	MainWindow.cpp
)

target_link_libraries(
    lcc-mouse
    PRIVATE libLumina ${QT}::Core ${QT}::Gui ${QT}::Widgets X11::X11 X11::Xi
)

install(TARGETS lcc-mouse)
install(FILES lcc-mouse.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
