# lumina-core/lumina-theme-engine/lthemeengine build, CMake edition. EXPERIMENTAL.
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2022 Adriaan de Groot <adridg@FreeBSD.org>

add_executable(
    lthemeengine
    main.cpp
    mainwindow.cpp
    tabpage.cpp
    appearancepage.cpp
    fontspage.cpp
    lthemeengine.cpp
    paletteeditdialog.cpp
    iconthemepage.cpp
    interfacepage.cpp
    fontconfigdialog.cpp
    qsspage.cpp
    qsseditordialog.cpp
    cursorthemepage.cpp
)
target_link_libraries(lthemeengine PRIVATE libLumina Qt5::Widgets)
target_compile_definitions(lthemeengine PRIVATE USE_WIDGETS)

install(TARGETS lthemeengine)
install(FILES lthemeengine.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
