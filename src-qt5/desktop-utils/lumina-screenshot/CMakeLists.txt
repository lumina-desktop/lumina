# Translation
set (TS_FILES
	i18n/l-screenshot_af.ts
	i18n/l-screenshot_ar.ts
	i18n/l-screenshot_az.ts
	i18n/l-screenshot_bg.ts
	i18n/l-screenshot_bn.ts
	i18n/l-screenshot_bs.ts
	i18n/l-screenshot_ca.ts
	i18n/l-screenshot_cs.ts
	i18n/l-screenshot_cy.ts
	i18n/l-screenshot_da.ts
	i18n/l-screenshot_de.ts
	i18n/l-screenshot_el.ts
	i18n/l-screenshot_en_GB.ts
	i18n/l-screenshot_en_ZA.ts
	i18n/l-screenshot_en_AU.ts
	i18n/l-screenshot_es.ts
	i18n/l-screenshot_et.ts
	i18n/l-screenshot_eu.ts
	i18n/l-screenshot_fa.ts
	i18n/l-screenshot_fi.ts
	i18n/l-screenshot_fr.ts
	i18n/l-screenshot_fr_CA.ts
	i18n/l-screenshot_gl.ts
	i18n/l-screenshot_he.ts
	i18n/l-screenshot_hi.ts
	i18n/l-screenshot_hr.ts
	i18n/l-screenshot_hu.ts
	i18n/l-screenshot_id.ts
	i18n/l-screenshot_is.ts
	i18n/l-screenshot_it.ts
	i18n/l-screenshot_ja.ts
	i18n/l-screenshot_ka.ts
	i18n/l-screenshot_ko.ts
	i18n/l-screenshot_lt.ts
	i18n/l-screenshot_lv.ts
	i18n/l-screenshot_mk.ts
	i18n/l-screenshot_mn.ts
	i18n/l-screenshot_ms.ts
	i18n/l-screenshot_mt.ts
	i18n/l-screenshot_nb.ts
	i18n/l-screenshot_nl.ts
	i18n/l-screenshot_pa.ts
	i18n/l-screenshot_pl.ts
	i18n/l-screenshot_pt.ts
	i18n/l-screenshot_pt_BR.ts
	i18n/l-screenshot_ro.ts
	i18n/l-screenshot_ru.ts
	i18n/l-screenshot_sk.ts
	i18n/l-screenshot_sl.ts
	i18n/l-screenshot_sr.ts
	i18n/l-screenshot_sv.ts
	i18n/l-screenshot_sw.ts
	i18n/l-screenshot_ta.ts
	i18n/l-screenshot_tg.ts
	i18n/l-screenshot_th.ts
	i18n/l-screenshot_tr.ts
	i18n/l-screenshot_uk.ts
	i18n/l-screenshot_uz.ts
	i18n/l-screenshot_vi.ts
	i18n/l-screenshot_zh_CN.ts
	i18n/l-screenshot_zh_HK.ts
	i18n/l-screenshot_zh_TW.ts
	i18n/l-screenshot_zu.ts
)

qt_add_translation(QM_FILES ${TS_FILES})

add_executable(lumina-screenshot
	main.cpp
	MainUI.cpp
	ImageEditor.cpp
	${TS_FILES}
	${QM_FILES}
)

add_custom_command(	
	TARGET lumina-screenshot
	COMMAND gzip -c ${CMAKE_CURRENT_SOURCE_DIR}/lumina-screenshot.1 > ${CMAKE_CURRENT_BINARY_DIR}/lumina-screenshot.1.gz
	OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/lumina-screenshot.1.gz
)

target_link_libraries(
    lumina-screenshot
    PRIVATE libLumina Qt5::Core Qt5::Gui Qt5::Widgets Qt5::X11Extras
)

install(TARGETS lumina-screenshot)
install(
	FILES ${QM_FILES}
	DESTINATION ${CMAKE_INSTALL_DATADIR}/lumina-desktop/i18n/
)
install(FILES lumina-screenshot.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(
	FILES ${CMAKE_CURRENT_BINARY_DIR}/lumina-screenshot.1.gz
	DESTINATION ${CMAKE_INSTALL_DATADIR}/man/man1
)
