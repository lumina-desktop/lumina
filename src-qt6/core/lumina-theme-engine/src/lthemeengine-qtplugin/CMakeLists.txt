# lumina-core/lumina-theme-engine/lthemeengine-qtplugin build, CMake edition. EXPERIMENTAL.
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2022 Adriaan de Groot <adridg@FreeBSD.org>

project(lthemeengine-qtplugin)

# The platform theme plugin subclasses QGenericUnixTheme, which is a
# private QtGui class, so it needs Qt6::GuiPrivate.
if(NOT LUMINA_HAS_GUIPRIVATE)
    message(STATUS "Skipping lthemeengine-qtplugin: Qt6 GuiPrivate not available")
    return()
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)

add_library(lthemeengine-qtplugin MODULE 
	main.cpp
	lthemeengineplatformtheme.cpp
	../lthemeengine/lthemeengine.cpp
)
set_target_properties(lthemeengine-qtplugin PROPERTIES OUTPUT_NAME lthemeengine)
target_link_libraries(lthemeengine-qtplugin PRIVATE
    Qt6::Widgets
    Qt6::GuiPrivate
    X11::X11
    X11::Xcursor
)
install(TARGETS lthemeengine-qtplugin DESTINATION ${PLUGINDIR}/platformthemes)
