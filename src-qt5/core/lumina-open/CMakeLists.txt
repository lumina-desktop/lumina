# lumina-core/lumina-open build, CMake edition. EXPERIMENTAL.
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2022 Adriaan de Groot <adridg@FreeBSD.org>

# Translation
set (TS_FILES
	i18n/lumina-open_af.ts
	i18n/lumina-open_ar.ts
	i18n/lumina-open_az.ts
	i18n/lumina-open_bg.ts
	i18n/lumina-open_bn.ts
	i18n/lumina-open_bs.ts
	i18n/lumina-open_ca.ts
	i18n/lumina-open_cs.ts
	i18n/lumina-open_cy.ts
	i18n/lumina-open_da.ts
	i18n/lumina-open_de.ts
	i18n/lumina-open_el.ts
	i18n/lumina-open_en_GB.ts
	i18n/lumina-open_en_ZA.ts
	i18n/lumina-open_en_AU.ts
	i18n/lumina-open_es.ts
	i18n/lumina-open_et.ts
	i18n/lumina-open_eu.ts
	i18n/lumina-open_fa.ts
	i18n/lumina-open_fi.ts
	i18n/lumina-open_fr.ts
	i18n/lumina-open_fr_CA.ts
	i18n/lumina-open_gl.ts
	i18n/lumina-open_he.ts
	i18n/lumina-open_hi.ts
	i18n/lumina-open_hr.ts
	i18n/lumina-open_hu.ts
	i18n/lumina-open_id.ts
	i18n/lumina-open_is.ts
	i18n/lumina-open_it.ts
	i18n/lumina-open_ja.ts
	i18n/lumina-open_ka.ts
	i18n/lumina-open_ko.ts
	i18n/lumina-open_lt.ts
	i18n/lumina-open_lv.ts
	i18n/lumina-open_mk.ts
	i18n/lumina-open_mn.ts
	i18n/lumina-open_ms.ts
	i18n/lumina-open_mt.ts
	i18n/lumina-open_nb.ts
	i18n/lumina-open_nl.ts
	i18n/lumina-open_pa.ts
	i18n/lumina-open_pl.ts
	i18n/lumina-open_pt.ts
	i18n/lumina-open_pt_BR.ts
	i18n/lumina-open_ro.ts
	i18n/lumina-open_ru.ts
	i18n/lumina-open_sk.ts
	i18n/lumina-open_sl.ts
	i18n/lumina-open_sr.ts
	i18n/lumina-open_sv.ts
	i18n/lumina-open_sw.ts
	i18n/lumina-open_ta.ts
	i18n/lumina-open_tg.ts
	i18n/lumina-open_th.ts
	i18n/lumina-open_tr.ts
	i18n/lumina-open_uk.ts
	i18n/lumina-open_uz.ts
	i18n/lumina-open_vi.ts
	i18n/lumina-open_zh_CN.ts
	i18n/lumina-open_zh_HK.ts
	i18n/lumina-open_zh_TW.ts
	i18n/lumina-open_zu.ts
)

qt_add_translation(QM_FILES ${TS_FILES})
qt_add_resources(RESOURCE_FILES lumina-open.qrc)

add_executable(lumina-open main.cpp LFileDialog.cpp ${RESOURCE_FILES} ${TS_FILES} ${QM_FILES})

add_custom_command(	
	TARGET lumina-open
	COMMAND gzip -c ${CMAKE_CURRENT_SOURCE_DIR}/lumina-open.1 > ${CMAKE_CURRENT_BINARY_DIR}/lumina-open.1.gz
	OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/lumina-open.1.gz
)

target_link_libraries(
    lumina-open
    PRIVATE libLumina Qt5::Core Qt5::Gui Qt5::Widgets Qt5::X11Extras
)

install(TARGETS lumina-open)

install(
	FILES ${QM_FILES}
	DESTINATION ${CMAKE_INSTALL_DATADIR}/lumina-desktop/i18n/
)

install(
	FILES ${CMAKE_CURRENT_BINARY_DIR}/lumina-open.1.gz
	DESTINATION ${CMAKE_INSTALL_DATADIR}/man/man1
)
