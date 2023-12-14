# Translation
set (TS_FILES
	i18n/lumina-xconfig_af.ts
	i18n/lumina-xconfig_ar.ts
	i18n/lumina-xconfig_az.ts
	i18n/lumina-xconfig_bg.ts
	i18n/lumina-xconfig_bn.ts
	i18n/lumina-xconfig_bs.ts
	i18n/lumina-xconfig_ca.ts
	i18n/lumina-xconfig_cs.ts
	i18n/lumina-xconfig_cy.ts
	i18n/lumina-xconfig_da.ts
	i18n/lumina-xconfig_de.ts
	i18n/lumina-xconfig_el.ts
	i18n/lumina-xconfig_en_GB.ts
	i18n/lumina-xconfig_en_ZA.ts
	i18n/lumina-xconfig_en_AU.ts
	i18n/lumina-xconfig_es.ts
	i18n/lumina-xconfig_et.ts
	i18n/lumina-xconfig_eu.ts
	i18n/lumina-xconfig_fa.ts
	i18n/lumina-xconfig_fi.ts
	i18n/lumina-xconfig_fr.ts
	i18n/lumina-xconfig_fr_CA.ts
	i18n/lumina-xconfig_gl.ts
	i18n/lumina-xconfig_he.ts
	i18n/lumina-xconfig_hi.ts
	i18n/lumina-xconfig_hr.ts
	i18n/lumina-xconfig_hu.ts
	i18n/lumina-xconfig_id.ts
	i18n/lumina-xconfig_is.ts
	i18n/lumina-xconfig_it.ts
	i18n/lumina-xconfig_ja.ts
	i18n/lumina-xconfig_ka.ts
	i18n/lumina-xconfig_ko.ts
	i18n/lumina-xconfig_lt.ts
	i18n/lumina-xconfig_lv.ts
	i18n/lumina-xconfig_mk.ts
	i18n/lumina-xconfig_mn.ts
	i18n/lumina-xconfig_ms.ts
	i18n/lumina-xconfig_mt.ts
	i18n/lumina-xconfig_nb.ts
	i18n/lumina-xconfig_nl.ts
	i18n/lumina-xconfig_pa.ts
	i18n/lumina-xconfig_pl.ts
	i18n/lumina-xconfig_pt.ts
	i18n/lumina-xconfig_pt_BR.ts
	i18n/lumina-xconfig_ro.ts
	i18n/lumina-xconfig_ru.ts
	i18n/lumina-xconfig_sk.ts
	i18n/lumina-xconfig_sl.ts
	i18n/lumina-xconfig_sr.ts
	i18n/lumina-xconfig_sv.ts
	i18n/lumina-xconfig_sw.ts
	i18n/lumina-xconfig_ta.ts
	i18n/lumina-xconfig_tg.ts
	i18n/lumina-xconfig_th.ts
	i18n/lumina-xconfig_tr.ts
	i18n/lumina-xconfig_uk.ts
	i18n/lumina-xconfig_uz.ts
	i18n/lumina-xconfig_vi.ts
	i18n/lumina-xconfig_zh_CN.ts
	i18n/lumina-xconfig_zh_HK.ts
	i18n/lumina-xconfig_zh_TW.ts
	i18n/lumina-xconfig_zu.ts
)

qt_add_translation(QM_FILES ${TS_FILES})

add_executable(lumina-xconfig
	main.cpp
	MainUI.cpp
	ScreenSettings.cpp
	${TS_FILES}
	${QM_FILES}
)

add_custom_command(	
	TARGET lumina-xconfig
	COMMAND gzip -c ${CMAKE_CURRENT_SOURCE_DIR}/lumina-xconfig.1 > ${CMAKE_CURRENT_BINARY_DIR}/lumina-xconfig.1.gz
	OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/lumina-xconfig.1.gz
)

target_link_libraries(
    lumina-xconfig
    PRIVATE libLumina Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Network
)

install(TARGETS lumina-xconfig)
install(
	FILES ${QM_FILES}
	DESTINATION ${CMAKE_INSTALL_DATADIR}/lumina-desktop/i18n/
)
install(FILES lumina-xconfig.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(
	FILES ${CMAKE_CURRENT_BINARY_DIR}/lumina-xconfig.1.gz
	DESTINATION ${CMAKE_INSTALL_DATADIR}/man/man1
)
