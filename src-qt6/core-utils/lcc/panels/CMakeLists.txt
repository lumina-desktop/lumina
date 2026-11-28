# lumina-core-utils/lcc/panels build
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2025 jt (q5sys) <jt@obs-sec.com>

# LCC Panels Module
# UI forms and moc are handled by CMAKE_AUTOUIC / CMAKE_AUTOMOC (set in src-qt6/CMakeLists.txt)

add_executable(lcc-panels
	main.cpp
	MainWindow.cpp
	PanelWidget.cpp
	LPlugins.cpp
	GetPluginDialog.cpp
	AppDialog.cpp
	ScriptDialog.cpp
)

target_link_libraries(
    lcc-panels
    PRIVATE libLumina ${QT}::Core ${QT}::Gui ${QT}::Widgets
)

lumina_add_manpage(lcc-panels lcc-panels.1 1)

install(TARGETS lcc-panels)
install(FILES lcc-panels.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
