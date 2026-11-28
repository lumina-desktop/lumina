# lumina-core-utils/lcc/sound-theme build
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2025 jt (q5sys) <jt@obs-sec.com>

# LCC Sound Theme Module
# UI forms and moc are handled by CMAKE_AUTOUIC / CMAKE_AUTOMOC (set in src-qt6/CMakeLists.txt)

add_executable(lcc-sound-theme
	main.cpp
	MainWindow.cpp
)

target_link_libraries(
    lcc-sound-theme
    PRIVATE libLumina ${QT}::Core ${QT}::Gui ${QT}::Widgets
)

lumina_add_manpage(lcc-sound-theme lcc-sound-theme.1 1)

install(TARGETS lcc-sound-theme)
install(FILES lcc-sound-theme.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
