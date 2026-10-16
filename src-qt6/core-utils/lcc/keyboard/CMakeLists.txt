# lumina-core-utils/lcc/keyboard build
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2025 jt (q5sys) <jt@obs-sec.com>

# LCC Keyboard Module
# UI forms and moc are handled by CMAKE_AUTOUIC / CMAKE_AUTOMOC (set in src-qt6/CMakeLists.txt)

add_executable(lcc-keyboard
	main.cpp
	MainWindow.cpp
)

target_link_libraries(
    lcc-keyboard
    PRIVATE libLumina ${QT}::Core ${QT}::Gui ${QT}::Widgets
)

install(TARGETS lcc-keyboard)
install(FILES lcc-keyboard.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
