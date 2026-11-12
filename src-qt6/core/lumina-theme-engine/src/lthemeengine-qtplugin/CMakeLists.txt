# lumina-core/lumina-theme-engine/lthemeengine-qtplugin build, CMake edition. EXPERIMENTAL.
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2022 Adriaan de Groot <adridg@FreeBSD.org>

project(lthemeengine-qtplugin)

# The Qt platform theme plugin requires ThemeSupportPrivate and GuiPrivate,
# which are Qt5-specific private modules. Qt6 does not have ThemeSupport.
# This component is deferred for Qt6 migration (Phase 5).
if(NOT LUMINA_HAS_THEMESUPPORT)
    message(STATUS "Skipping lthemeengine-qtplugin: ThemeSupport not available (Qt6 build)")
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
    ${QT}::DBus
    Qt5::ThemeSupportPrivate
    Qt5::X11Extras
    X11::Xcursor
)
install(TARGETS lthemeengine-qtplugin DESTINATION ${PLUGINDIR}/platformthemes)
