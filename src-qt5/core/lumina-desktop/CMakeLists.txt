# lumina-core/lumina-desktop build, CMake edition. EXPERIMENTAL.
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2022 Adriaan de Groot <adridg@FreeBSD.org>

# The plugins, as well as sources in this directory, expect to
# include from this directory, from src-screensaver/, but **also* from
# the top-level (outside of lumina-core entirely!) src-cpp.
set(TOPLEVEL_SRC_CPP ${CMAKE_SOURCE_DIR}/../src-cpp/)
include_directories(
    lumina-desktop
    PRIVATE
    ${TOPLEVEL_SRC_CPP}
    ${CMAKE_CURRENT_LIST_DIR}
    src-screensaver
)

add_subdirectory(desktop-plugins)
add_subdirectory(panel-plugins)
add_subdirectory(src-screensaver)

# Translation
set (TS_FILES
	i18n/lumina-desktop_af.ts
	i18n/lumina-desktop_ar.ts
	i18n/lumina-desktop_az.ts
	i18n/lumina-desktop_bg.ts
	i18n/lumina-desktop_bn.ts
	i18n/lumina-desktop_bs.ts
	i18n/lumina-desktop_ca.ts
	i18n/lumina-desktop_cs.ts
	i18n/lumina-desktop_cy.ts
	i18n/lumina-desktop_da.ts
	i18n/lumina-desktop_de.ts
	i18n/lumina-desktop_el.ts
	i18n/lumina-desktop_en_AU.ts
	i18n/lumina-desktop_en_GB.ts
	i18n/lumina-desktop_en_ZA.ts
	i18n/lumina-desktop_es.ts
	i18n/lumina-desktop_et.ts
	i18n/lumina-desktop_eu.ts
	i18n/lumina-desktop_fa.ts
	i18n/lumina-desktop_fi.ts
	i18n/lumina-desktop_fr.ts
	i18n/lumina-desktop_fr_CA.ts
	i18n/lumina-desktop_gl.ts
	i18n/lumina-desktop_he.ts
	i18n/lumina-desktop_hi.ts
	i18n/lumina-desktop_hr.ts
	i18n/lumina-desktop_hu.ts
	i18n/lumina-desktop_id.ts
	i18n/lumina-desktop_is.ts
	i18n/lumina-desktop_it.ts
	i18n/lumina-desktop_ja.ts
	i18n/lumina-desktop_ka.ts
	i18n/lumina-desktop_ko.ts
	i18n/lumina-desktop_lt.ts
	i18n/lumina-desktop_lv.ts
	i18n/lumina-desktop_mk.ts
	i18n/lumina-desktop_mn.ts
	i18n/lumina-desktop_ms.ts
	i18n/lumina-desktop_mt.ts
	i18n/lumina-desktop_nb.ts
	i18n/lumina-desktop_nl.ts
	i18n/lumina-desktop_pa.ts
	i18n/lumina-desktop_pl.ts
	i18n/lumina-desktop_pt.ts
	i18n/lumina-desktop_pt_BR.ts
	i18n/lumina-desktop_ro.ts
	i18n/lumina-desktop_ru.ts
	i18n/lumina-desktop_sk.ts
	i18n/lumina-desktop_sl.ts
	i18n/lumina-desktop_sr.ts
	i18n/lumina-desktop_sv.ts
	i18n/lumina-desktop_sw.ts
	i18n/lumina-desktop_ta.ts
	i18n/lumina-desktop_tg.ts
	i18n/lumina-desktop_th.ts
	i18n/lumina-desktop_tr.ts
	i18n/lumina-desktop_uk.ts
	i18n/lumina-desktop_uz.ts
	i18n/lumina-desktop_vi.ts
	i18n/lumina-desktop_zh_CN.ts
	i18n/lumina-desktop_zh_HK.ts
	i18n/lumina-desktop_zh_TW.ts
	i18n/lumina-desktop_zu.ts
)

qt_add_translation(QM_FILES ${TS_FILES})
qt_add_resources(RESOURCE_FILES Lumina-DE.qrc)

add_executable(
    lumina-desktop
    main.cpp
    WMProcess.cpp
    LXcbEventFilter.cpp
    LSession.cpp
    LDesktop.cpp
    LDesktopBackground.cpp
    LDesktopPluginSpace.cpp
    LPanel.cpp
    LWinInfo.cpp
    AppMenu.cpp
    SettingsMenu.cpp
    SystemWindow.cpp
    BootSplash.cpp
    # Header-only classes
    JsonMenu.h
    ${RESOURCE_FILES}
    ${TM_FILES}
    ${QM_FILES}
)
# FIXME: because we're CMake-ifying only core,
# the src-cpp bits lie "outside" the regular source tree.
# Make that a library at some point.
target_sources(
    lumina-desktop
    PUBLIC
        ${TOPLEVEL_SRC_CPP}/plugins-base.cpp
        ${TOPLEVEL_SRC_CPP}/plugins-desktop.cpp
        ${TOPLEVEL_SRC_CPP}/plugins-screensaver.cpp
)

add_custom_command(	
	TARGET lumina-desktop
	COMMAND gzip -c ${CMAKE_CURRENT_SOURCE_DIR}/lumina-desktop.1 > ${CMAKE_CURRENT_BINARY_DIR}/lumina-desktop.1.gz
	OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/lumina-desktop.1.gz
)

# Picks up many Qt libraries from libLumina's PUBLIC settings
target_link_libraries(
    lumina-desktop
    PRIVATE
        libLumina
        Qt5::Gui
        Qt5::Network
        Qt5::Quick
        Qt5::Qml
        XCB::XCB
        XCB::COMPOSITE
        XCB::DAMAGE
        XCB::DPMS
        XCB::EWMH
        XCB::ICCCM
        XCB::IMAGE
)
# The panel-plugins have their own static library
target_link_libraries(
    lumina-desktop
    PRIVATE integrated-desktop integrated-panel integrated-screensaver
)


install(TARGETS lumina-desktop)

# Additional files
install(
    FILES
        audiofiles/Login.ogg
        audiofiles/Logout.ogg
        audiofiles/low-battery.ogg
        extrafiles/Lumina-DE.png
        fluxboxconf/fluxbox-init-rc
        fluxboxconf/fluxbox-keys
        fluxboxconf/theme.cfg
        defaults/luminaDesktop.conf
        defaults/luminaDesktop-TrueOS.conf
        defaults/compton.conf
        defaults/desktop-background.jpg
    DESTINATION ${CMAKE_INSTALL_DATADIR}/lumina-desktop/
)
install(FILES Lumina-DE.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/xsessions/)
install(
    FILES Lumina-DE.svg
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps/
)
install(
	FILES ${QM_FILES}
	DESTINATION ${CMAKE_INSTALL_DATADIR}/lumina-desktop/i18n/
)

install(
	FILES ${CMAKE_CURRENT_BINARY_DIR}/lumina-desktop.1.gz
	DESTINATION ${CMAKE_INSTALL_DATADIR}/man/man1
)
