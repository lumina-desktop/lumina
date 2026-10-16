# lumina-core-utils/lcc/wallpaper build
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2025 jt (q5sys) <jt@obs-sec.com>

# LCC Wallpaper Module - Configure desktop wallpaper settings
# UI forms and moc are handled by CMAKE_AUTOUIC / CMAKE_AUTOMOC (set in src-qt6/CMakeLists.txt)

add_executable(lcc-wallpaper
	main.cpp
	MainWindow.cpp
)

target_link_libraries(
    lcc-wallpaper
    PRIVATE libLumina ${QT}::Core ${QT}::Gui ${QT}::Widgets
)

install(TARGETS lcc-wallpaper)
install(FILES lcc-wallpaper.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
