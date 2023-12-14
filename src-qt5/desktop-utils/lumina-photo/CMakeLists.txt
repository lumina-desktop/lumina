set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Translation
set (TS_FILES
	i18n/l-photo_af.ts
	i18n/l-photo_ar.ts
	i18n/l-photo_az.ts
	i18n/l-photo_bg.ts
	i18n/l-photo_bn.ts
	i18n/l-photo_bs.ts
	i18n/l-photo_ca.ts
	i18n/l-photo_cs.ts
	i18n/l-photo_cy.ts
	i18n/l-photo_da.ts
	i18n/l-photo_de.ts
	i18n/l-photo_el.ts
	i18n/l-photo_en_GB.ts
	i18n/l-photo_en_ZA.ts
	i18n/l-photo_en_AU.ts
	i18n/l-photo_es.ts
	i18n/l-photo_et.ts
	i18n/l-photo_eu.ts
	i18n/l-photo_fa.ts
	i18n/l-photo_fi.ts
	i18n/l-photo_fr.ts
	i18n/l-photo_fr_CA.ts
	i18n/l-photo_gl.ts
	i18n/l-photo_he.ts
	i18n/l-photo_hi.ts
	i18n/l-photo_hr.ts
	i18n/l-photo_hu.ts
	i18n/l-photo_id.ts
	i18n/l-photo_is.ts
	i18n/l-photo_it.ts
	i18n/l-photo_ja.ts
	i18n/l-photo_ka.ts
	i18n/l-photo_ko.ts
	i18n/l-photo_lt.ts
	i18n/l-photo_lv.ts
	i18n/l-photo_mk.ts
	i18n/l-photo_mn.ts
	i18n/l-photo_ms.ts
	i18n/l-photo_mt.ts
	i18n/l-photo_nb.ts
	i18n/l-photo_nl.ts
	i18n/l-photo_pa.ts
	i18n/l-photo_pl.ts
	i18n/l-photo_pt.ts
	i18n/l-photo_pt_BR.ts
	i18n/l-photo_ro.ts
	i18n/l-photo_ru.ts
	i18n/l-photo_sk.ts
	i18n/l-photo_sl.ts
	i18n/l-photo_sr.ts
	i18n/l-photo_sv.ts
	i18n/l-photo_sw.ts
	i18n/l-photo_ta.ts
	i18n/l-photo_tg.ts
	i18n/l-photo_th.ts
	i18n/l-photo_tr.ts
	i18n/l-photo_uk.ts
	i18n/l-photo_uz.ts
	i18n/l-photo_vi.ts
	i18n/l-photo_zh_CN.ts
	i18n/l-photo_zh_HK.ts
	i18n/l-photo_zh_TW.ts
	i18n/l-photo_zu.ts
)

qt_add_translation(QM_FILES ${TS_FILES})

add_executable(lumina-photo
	main.cpp
	mainUI.cpp
	PhotoView.cpp
	${TS_FILES}
	${QM_FILES}
)

add_custom_command(	
	TARGET lumina-photo
	COMMAND gzip -c ${CMAKE_CURRENT_SOURCE_DIR}/lumina-photo.1 > ${CMAKE_CURRENT_BINARY_DIR}/lumina-photo.1.gz
	OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/lumina-photo.1.gz
)

target_link_libraries(
    lumina-photo
    PRIVATE libLumina Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Svg Qt5::PrintSupport
)

install(TARGETS lumina-photo)
install(
	FILES ${QM_FILES}
	DESTINATION ${CMAKE_INSTALL_DATADIR}/lumina-desktop/i18n/
)
install(FILES lumina-photo.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(
	FILES ${CMAKE_CURRENT_BINARY_DIR}/lumina-photo.1.gz
	DESTINATION ${CMAKE_INSTALL_DATADIR}/man/man1
)
