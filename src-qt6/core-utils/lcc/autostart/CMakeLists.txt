# lumina-core-utils/lcc/autostart build
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2025 jt (q5sys) <jt@obs-sec.com>

# LCC Autostart Module - Configure applications that start with the desktop
# UI forms and moc are handled by CMAKE_AUTOUIC / CMAKE_AUTOMOC (set in src-qt6/CMakeLists.txt)

add_executable(lcc-autostart
	main.cpp
	MainWindow.cpp
)

target_link_libraries(
    lcc-autostart
    PRIVATE libLumina ${QT}::Core ${QT}::Gui ${QT}::Widgets
)

lumina_add_manpage(lcc-autostart lcc-autostart.1 1)

install(TARGETS lcc-autostart)
install(FILES lcc-autostart.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
