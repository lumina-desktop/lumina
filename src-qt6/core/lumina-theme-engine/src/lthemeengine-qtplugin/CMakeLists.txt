# lumina-core/lumina-theme-engine/lthemeengine-qtplugin build, CMake edition. EXPERIMENTAL.
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2022 Adriaan de Groot <adridg@FreeBSD.org>

project(lthemeengine-qtplugin)

# The platform theme plugin subclasses QGenericUnixTheme, which is a
# private QtGui class, so it needs GuiPrivate (Qt5: ThemeSupport too).
# Without it Lumina's theme settings do nothing, so say so loudly instead of a STATUS line.
if(NOT LUMINA_HAS_GUIPRIVATE)
    message(WARNING "Skipping lthemeengine-qtplugin: ${QT} GuiPrivate (private QtGui headers/CMake target) not found - Lumina theming will not be applied to Qt applications")
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
    ${QT}::Widgets
    ${QT}::GuiPrivate
    X11::X11
    X11::Xcursor
)
if(NOT WITH_QT6)
    target_link_libraries(lthemeengine-qtplugin PRIVATE Qt5::ThemeSupportPrivate Qt5::X11Extras)
endif()
install(TARGETS lthemeengine-qtplugin DESTINATION ${PLUGINDIR}/platformthemes)
