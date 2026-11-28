# lumina-core-utils/lcc/window-manager build
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2025 jt (q5sys) <jt@obs-sec.com>

# LCC Window Manager Module
# UI forms and moc are handled by CMAKE_AUTOUIC / CMAKE_AUTOMOC (set in src-qt6/CMakeLists.txt)

add_executable(lcc-window-manager
	main.cpp
	MainWindow.cpp
)

target_link_libraries(
    lcc-window-manager
    PRIVATE libLumina ${QT}::Core ${QT}::Gui ${QT}::Widgets
)

lumina_add_manpage(lcc-window-manager lcc-window-manager.1 1)

install(TARGETS lcc-window-manager)
install(FILES lcc-window-manager.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
