# lumina-core/lumina-theme-engine/lthemeengine-qtplugin build, CMake edition. EXPERIMENTAL.
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2022 Adriaan de Groot <adridg@FreeBSD.org>

project(lthemeengine-qtplugin)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)

add_library(lthemeengine-qtplugin MODULE 
	main.cpp
	lthemeengineplatformtheme.cpp
	../lthemeengine/lthemeengine.cpp
)
set_target_properties(lthemeengine-qtplugin PROPERTIES OUTPUT_NAME lthemeengine)
target_link_libraries(lthemeengine-qtplugin PRIVATE Qt5::Widgets Qt5::GuiPrivate Qt5::Widgets Qt5::DBus Qt5::ThemeSupportPrivate Qt5::X11Extras X11::Xcursor)
install(TARGETS lthemeengine-qtplugin DESTINATION ${PLUGINDIR}/platformthemes)
