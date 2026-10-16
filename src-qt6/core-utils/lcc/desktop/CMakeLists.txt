# lumina-core-utils/lcc/desktop build
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2025 jt (q5sys) <jt@obs-sec.com>

# LCC Desktop Module
# UI forms and moc are handled by CMAKE_AUTOUIC / CMAKE_AUTOMOC (set in src-qt6/CMakeLists.txt)

add_executable(lcc-desktop
	main.cpp
	MainWindow.cpp
)

target_link_libraries(
    lcc-desktop
    PRIVATE libLumina ${QT}::Core ${QT}::Gui ${QT}::Widgets
)

add_custom_command(
	TARGET lcc-desktop
	POST_BUILD
	COMMAND gzip -c ${CMAKE_CURRENT_SOURCE_DIR}/lcc-desktop.1 > ${CMAKE_CURRENT_BINARY_DIR}/lcc-desktop.1.gz
	BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/lcc-desktop.1.gz
)

install(TARGETS lcc-desktop)
install(FILES lcc-desktop.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(
	FILES ${CMAKE_CURRENT_BINARY_DIR}/lcc-desktop.1.gz
	DESTINATION ${CMAKE_INSTALL_DATADIR}/man/man1
)
