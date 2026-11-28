# lumina-core-utils/lcc/main build
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2025 jt (q5sys) <jt@obs-sec.com>

# Main LCC (Lumina Control Center) launcher executable
# UI forms and moc are handled by CMAKE_AUTOUIC / CMAKE_AUTOMOC (set in src-qt6/CMakeLists.txt)

add_executable(lcc
	main.cpp
	MainWindow.cpp
)

target_link_libraries(
    lcc
    PRIVATE libLumina ${QT}::Core ${QT}::Gui ${QT}::Widgets
)

lumina_add_manpage(lcc lcc.1 1)

install(TARGETS lcc)
install(FILES lcc.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
