# lumina-core-utils/lcc/menu build
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2025 jt (q5sys) <jt@obs-sec.com>

# LCC Menu Module
# UI forms and moc are handled by CMAKE_AUTOUIC / CMAKE_AUTOMOC (set in src-qt6/CMakeLists.txt)

add_executable(lcc-menu
	main.cpp
	MainWindow.cpp
	LPlugins.cpp
	GetPluginDialog.cpp
	AppDialog.cpp
	ScriptDialog.cpp
)

target_link_libraries(
    lcc-menu
    PRIVATE libLumina ${QT}::Core ${QT}::Gui ${QT}::Widgets
)

install(TARGETS lcc-menu)
install(FILES lcc-menu.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
