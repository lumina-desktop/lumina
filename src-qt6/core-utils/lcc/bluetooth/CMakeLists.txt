# lumina-core-utils/lcc/bluetooth build
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2025 jt (q5sys) <jt@obs-sec.com>

# LCC Bluetooth Module
# UI forms and moc are handled by CMAKE_AUTOUIC / CMAKE_AUTOMOC (set in src-qt6/CMakeLists.txt)

add_executable(lcc-bluetooth
	main.cpp
	MainWindow.cpp
)

target_link_libraries(
    lcc-bluetooth
    PRIVATE libLumina ${QT}::Core ${QT}::Gui ${QT}::Widgets
)

lumina_add_manpage(lcc-bluetooth lcc-bluetooth.1 1)

install(TARGETS lcc-bluetooth)
install(FILES lcc-bluetooth.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
