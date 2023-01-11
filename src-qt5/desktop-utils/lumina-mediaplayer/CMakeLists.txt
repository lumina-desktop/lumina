# Translation
set (TS_FILES
	i18n/l-mediap_af.ts
	i18n/l-mediap_ar.ts
	i18n/l-mediap_az.ts
	i18n/l-mediap_bg.ts
	i18n/l-mediap_bn.ts
	i18n/l-mediap_bs.ts
	i18n/l-mediap_ca.ts
	i18n/l-mediap_cs.ts
	i18n/l-mediap_cy.ts
	i18n/l-mediap_da.ts
	i18n/l-mediap_de.ts
	i18n/l-mediap_el.ts
	i18n/l-mediap_en_GB.ts
	i18n/l-mediap_en_ZA.ts
	i18n/l-mediap_en_AU.ts
	i18n/l-mediap_es.ts
	i18n/l-mediap_et.ts
	i18n/l-mediap_eu.ts
	i18n/l-mediap_fa.ts
	i18n/l-mediap_fi.ts
	i18n/l-mediap_fr.ts
	i18n/l-mediap_fr_CA.ts
	i18n/l-mediap_gl.ts
	i18n/l-mediap_he.ts
	i18n/l-mediap_hi.ts
	i18n/l-mediap_hr.ts
	i18n/l-mediap_hu.ts
	i18n/l-mediap_id.ts
	i18n/l-mediap_is.ts
	i18n/l-mediap_it.ts
	i18n/l-mediap_ja.ts
	i18n/l-mediap_ka.ts
	i18n/l-mediap_ko.ts
	i18n/l-mediap_lt.ts
	i18n/l-mediap_lv.ts
	i18n/l-mediap_mk.ts
	i18n/l-mediap_mn.ts
	i18n/l-mediap_ms.ts
	i18n/l-mediap_mt.ts
	i18n/l-mediap_nb.ts
	i18n/l-mediap_nl.ts
	i18n/l-mediap_pa.ts
	i18n/l-mediap_pl.ts
	i18n/l-mediap_pt.ts
	i18n/l-mediap_pt_BR.ts
	i18n/l-mediap_ro.ts
	i18n/l-mediap_ru.ts
	i18n/l-mediap_sk.ts
	i18n/l-mediap_sl.ts
	i18n/l-mediap_sr.ts
	i18n/l-mediap_sv.ts
	i18n/l-mediap_sw.ts
	i18n/l-mediap_ta.ts
	i18n/l-mediap_tg.ts
	i18n/l-mediap_th.ts
	i18n/l-mediap_tr.ts
	i18n/l-mediap_uk.ts
	i18n/l-mediap_uz.ts
	i18n/l-mediap_vi.ts
	i18n/l-mediap_zh_CN.ts
	i18n/l-mediap_zh_HK.ts
	i18n/l-mediap_zh_TW.ts
	i18n/l-mediap_zu.ts
)

qt_add_translation(QM_FILES ${TS_FILES})

# TODO: Fix resource error, it seems cmake doesn't like parsing attributes.
#qt_add_resources(RESOURCE_FILES extra/resources.qrc)

add_executable(lumina-mediaplayer
	main.cpp
	mainUI.cpp
	PianoBarProcess.cpp
	${RESOURCE_FILES}
	${TS_FILES}
	${QM_FILES}
)

add_custom_command(	
	TARGET lumina-mediaplayer
	COMMAND gzip -c ${CMAKE_CURRENT_SOURCE_DIR}/lumina-mediaplayer.1 > ${CMAKE_CURRENT_BINARY_DIR}/lumina-mediaplayer.1.gz
	OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/lumina-mediaplayer.1.gz
)

target_link_libraries(
    lumina-mediaplayer
    PRIVATE libLumina Qt5::Core Qt5::Gui Qt5::Widgets Qt5::MultimediaWidgets Qt5::Multimedia Qt5::Svg
)

install(TARGETS lumina-mediaplayer)
install(
	FILES ${QM_FILES}
	DESTINATION ${CMAKE_INSTALL_DATADIR}/lumina-desktop/i18n/
)

install(FILES lumina-mediaplayer.desktop lumina-mediaplayer-pandora.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(
	FILES ${CMAKE_CURRENT_BINARY_DIR}/lumina-mediaplayer.1.gz
	DESTINATION ${CMAKE_INSTALL_DATADIR}/man/man1
)
