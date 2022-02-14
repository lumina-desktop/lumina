# lumina-core/icon-themes build, CMake edition. EXPERIMENTAL.
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2022 Adriaan de Groot <adridg@FreeBSD.org>

include(CMakeParseArguments)
function(install_theme)
    set(options "") # None
    set(oneValueArgs NAME)
    set(multiValueArgs TOPLEVEL SUBDIRS)

    cmake_parse_arguments(
        THEME
        "${options}"
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN}
    )

    if(NOT THEME_NAME)
        message(FATAL_ERROR "Missing theme name")
    endif()

    set(target_directory "${CMAKE_INSTALL_DATADIR}/icons/${THEME_NAME}")

    install(FILES ${THEME_TOPLEVEL} DESTINATION ${target_directory})
    foreach(d ${THEME_SUBDIRS})
        file(GLOB_RECURSE files "${d}/*.svg")
        install(FILES ${files} DESTINATION ${target_directory}/${d})
    endforeach()
endfunction()

add_subdirectory(lumina-icons)
# add_subdirectory(material-design-light)
# add_subdirectory(material-design-dark)
