# lumina-core-utils/lcc/compositor build
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2025 jt (q5sys) <jt@obs-sec.com>

# LCC Compositor Module
# UI forms and moc are handled by CMAKE_AUTOUIC / CMAKE_AUTOMOC (set in src-qt6/CMakeLists.txt)

add_executable(lcc-compositor
	main.cpp
	MainWindow.cpp
)

target_link_libraries(
    lcc-compositor
    PRIVATE libLumina ${QT}::Core ${QT}::Gui ${QT}::Widgets
)

lumina_add_manpage(lcc-compositor lcc-compositor.1 1)

install(TARGETS lcc-compositor)
install(FILES lcc-compositor.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
