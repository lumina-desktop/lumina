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

add_custom_command(
	TARGET lcc-window-manager
	POST_BUILD
	COMMAND gzip -c ${CMAKE_CURRENT_SOURCE_DIR}/lcc-window-manager.1 > ${CMAKE_CURRENT_BINARY_DIR}/lcc-window-manager.1.gz
	BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/lcc-window-manager.1.gz
)

install(TARGETS lcc-window-manager)
install(FILES lcc-window-manager.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(
	FILES ${CMAKE_CURRENT_BINARY_DIR}/lcc-window-manager.1.gz
	DESTINATION ${CMAKE_INSTALL_DATADIR}/man/man1
)
