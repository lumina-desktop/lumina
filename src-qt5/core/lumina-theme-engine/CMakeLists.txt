# lumina-core/lumina-theme-engine build, CMake edition. EXPERIMENTAL.
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2022 Adriaan de Groot <adridg@FreeBSD.org>

get_target_property(QT_LRELEASE_EXECUTABLE Qt5::lrelease IMPORTED_LOCATION)

if(QT_LRELEASE_EXECUTABLE)
    message(STATUS "Found lrelease executable: " ${QT_LRELEASE_EXECUTABLE})
else()
    message(FATAL_ERROR "Could NOT find lrelease executable")
endif()

get_filename_component(QT_QTPATHS_EXECUTABLE ${QT_LRELEASE_EXECUTABLE} DIRECTORY)
set(QT_QTPATHS_EXECUTABLE ${QT_QTPATHS_EXECUTABLE}/qtpaths)

if(EXISTS ${QT_QTPATHS_EXECUTABLE})
    message(STATUS "Found qtpaths executable: " ${QT_QTPATHS_EXECUTABLE})
else()
    message(FATAL_ERROR "Could NOT find qtpaths executable")
endif()

execute_process(COMMAND ${QT_QTPATHS_EXECUTABLE} --plugin-dir OUTPUT_VARIABLE PLUGINDIR OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "Plugin path: " ${PLUGINDIR})

add_subdirectory(src/lthemeengine)
add_subdirectory(src/lthemeengine-qtplugin)
add_subdirectory(src/lthemeengine-sstest)
add_subdirectory(src/lthemeengine-style)

install(
    DIRECTORY colors
    DESTINATION ${CMAKE_INSTALL_DATADIR}/lthemeengine/colors
    FILES_MATCHING
    PATTERN *.conf
)
install(
    DIRECTORY qss
    DESTINATION ${CMAKE_INSTALL_DATADIR}/lthemeengine/qss
    FILES_MATCHING
    PATTERN *.qss
)
install(
    DIRECTORY desktop_qss
    DESTINATION ${CMAKE_INSTALL_DATADIR}/lthemeengine/desktop_qss
    FILES_MATCHING
    PATTERN *.qss
)
