# lumina-core/lumina-theme-engine/lthemeengine-style build, CMake edition. EXPERIMENTAL.
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2022 Adriaan de Groot <adridg@FreeBSD.org>

project(lthemeengine-style)

add_definitions(-DUSE_WIDGETS)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)

add_library(lthemeengine-style MODULE 
	plugin.cpp
	lthemeengineproxystyle.cpp
	../lthemeengine/lthemeengine.cpp
)
target_link_libraries(lthemeengine-style PRIVATE Qt5::Widgets)
install(TARGETS lthemeengine-style DESTINATION ${PLUGINDIR}/styles) 
