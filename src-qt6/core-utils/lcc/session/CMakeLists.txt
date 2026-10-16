# lumina-core-utils/lcc/session build
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2025 jt (q5sys) <jt@obs-sec.com>

# LCC Session Module
# UI forms and moc are handled by CMAKE_AUTOUIC / CMAKE_AUTOMOC (set in src-qt6/CMakeLists.txt)

add_executable(lcc-session
	main.cpp
	MainWindow.cpp
)

target_link_libraries(
    lcc-session
    PRIVATE libLumina ${QT}::Core ${QT}::Gui ${QT}::Widgets
)

install(TARGETS lcc-session)
install(FILES lcc-session.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
