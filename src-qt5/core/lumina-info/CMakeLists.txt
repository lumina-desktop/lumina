# lumina-core/lumina-open build, CMake edition. EXPERIMENTAL.
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2022 Adriaan de Groot <adridg@FreeBSD.org>

# Translation
set (TS_FILES
	i18n/lumina-info_af.ts
	i18n/lumina-info_ar.ts
	i18n/lumina-info_az.ts
	i18n/lumina-info_bg.ts
	i18n/lumina-info_bn.ts
	i18n/lumina-info_bs.ts
	i18n/lumina-info_ca.ts
	i18n/lumina-info_cs.ts
	i18n/lumina-info_cy.ts
	i18n/lumina-info_da.ts
	i18n/lumina-info_de.ts
	i18n/lumina-info_el.ts
	i18n/lumina-info_en_GB.ts
	i18n/lumina-info_en_ZA.ts
	i18n/lumina-info_en_AU.ts
	i18n/lumina-info_es.ts
	i18n/lumina-info_et.ts
	i18n/lumina-info_eu.ts
	i18n/lumina-info_fa.ts
	i18n/lumina-info_fi.ts
	i18n/lumina-info_fr.ts
	i18n/lumina-info_fr_CA.ts
	i18n/lumina-info_gl.ts
	i18n/lumina-info_he.ts
	i18n/lumina-info_hi.ts
	i18n/lumina-info_hr.ts
	i18n/lumina-info_hu.ts
	i18n/lumina-info_id.ts
	i18n/lumina-info_is.ts
	i18n/lumina-info_it.ts
	i18n/lumina-info_ja.ts
	i18n/lumina-info_ka.ts
	i18n/lumina-info_ko.ts
	i18n/lumina-info_lt.ts
	i18n/lumina-info_lv.ts
	i18n/lumina-info_mk.ts
	i18n/lumina-info_mn.ts
	i18n/lumina-info_ms.ts
	i18n/lumina-info_mt.ts
	i18n/lumina-info_nb.ts
	i18n/lumina-info_nl.ts
	i18n/lumina-info_pa.ts
	i18n/lumina-info_pl.ts
	i18n/lumina-info_pt.ts
	i18n/lumina-info_pt_BR.ts
	i18n/lumina-info_ro.ts
	i18n/lumina-info_ru.ts
	i18n/lumina-info_sk.ts
	i18n/lumina-info_sl.ts
	i18n/lumina-info_sr.ts
	i18n/lumina-info_sv.ts
	i18n/lumina-info_sw.ts
	i18n/lumina-info_ta.ts
	i18n/lumina-info_tg.ts
	i18n/lumina-info_th.ts
	i18n/lumina-info_tr.ts
	i18n/lumina-info_uk.ts
	i18n/lumina-info_uz.ts
	i18n/lumina-info_vi.ts
	i18n/lumina-info_zh_CN.ts
	i18n/lumina-info_zh_HK.ts
	i18n/lumina-info_zh_TW.ts
	i18n/lumina-info_zu.ts
)

qt_add_translation(QM_FILES ${TS_FILES})
qt_add_resources(RESOURCE_FILES lumina-info.qrc)

add_executable(lumina-info main.cpp MainUI.cpp ${RESOURCE_FILES} ${TS_FILES} ${QM_FILES})

add_custom_command(	
	TARGET lumina-info
	COMMAND gzip -c ${CMAKE_CURRENT_SOURCE_DIR}/lumina-info.1 > ${CMAKE_CURRENT_BINARY_DIR}/lumina-info.1.gz
	OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/lumina-info.1.gz
)

target_link_libraries(
    lumina-info
    PRIVATE libLumina Qt5::Core Qt5::Gui Qt5::Widgets Qt5::X11Extras
)
install(TARGETS lumina-info)
install(
	FILES ${QM_FILES}
	DESTINATION ${CMAKE_INSTALL_DATADIR}/lumina-desktop/i18n/
)
install(
	FILES lumina-info.desktop lumina-support.desktop
	DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)

install(
	FILES ${CMAKE_CURRENT_BINARY_DIR}/lumina-info.1.gz
	DESTINATION ${CMAKE_INSTALL_DATADIR}/man/man1
)
