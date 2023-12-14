# Translation
set (TS_FILES
	i18n/lumina-fm_af.ts
	i18n/lumina-fm_ar.ts
	i18n/lumina-fm_az.ts
	i18n/lumina-fm_bg.ts
	i18n/lumina-fm_bn.ts
	i18n/lumina-fm_bs.ts
	i18n/lumina-fm_ca.ts
	i18n/lumina-fm_cs.ts
	i18n/lumina-fm_cy.ts
	i18n/lumina-fm_da.ts
	i18n/lumina-fm_de.ts
	i18n/lumina-fm_el.ts
	i18n/lumina-fm_en_GB.ts
	i18n/lumina-fm_en_ZA.ts
	i18n/lumina-fm_en_AU.ts
	i18n/lumina-fm_es.ts
	i18n/lumina-fm_et.ts
	i18n/lumina-fm_eu.ts
	i18n/lumina-fm_fa.ts
	i18n/lumina-fm_fi.ts
	i18n/lumina-fm_fr.ts
	i18n/lumina-fm_fr_CA.ts
	i18n/lumina-fm_gl.ts
	i18n/lumina-fm_he.ts
	i18n/lumina-fm_hi.ts
	i18n/lumina-fm_hr.ts
	i18n/lumina-fm_hu.ts
	i18n/lumina-fm_id.ts
	i18n/lumina-fm_is.ts
	i18n/lumina-fm_it.ts
	i18n/lumina-fm_ja.ts
	i18n/lumina-fm_ka.ts
	i18n/lumina-fm_ko.ts
	i18n/lumina-fm_lt.ts
	i18n/lumina-fm_lv.ts
	i18n/lumina-fm_mk.ts
	i18n/lumina-fm_mn.ts
	i18n/lumina-fm_ms.ts
	i18n/lumina-fm_mt.ts
	i18n/lumina-fm_nb.ts
	i18n/lumina-fm_nl.ts
	i18n/lumina-fm_pa.ts
	i18n/lumina-fm_pl.ts
	i18n/lumina-fm_pt.ts
	i18n/lumina-fm_pt_BR.ts
	i18n/lumina-fm_ro.ts
	i18n/lumina-fm_ru.ts
	i18n/lumina-fm_sk.ts
	i18n/lumina-fm_sl.ts
	i18n/lumina-fm_sr.ts
	i18n/lumina-fm_sv.ts
	i18n/lumina-fm_sw.ts
	i18n/lumina-fm_ta.ts
	i18n/lumina-fm_tg.ts
	i18n/lumina-fm_th.ts
	i18n/lumina-fm_tr.ts
	i18n/lumina-fm_uk.ts
	i18n/lumina-fm_uz.ts
	i18n/lumina-fm_vi.ts
	i18n/lumina-fm_zh_CN.ts
	i18n/lumina-fm_zh_HK.ts
	i18n/lumina-fm_zh_TW.ts
	i18n/lumina-fm_zu.ts
)

qt_add_translation(QM_FILES ${TS_FILES})

add_executable(lumina-fm
	main.cpp
	MainUI.cpp
	ScrollDialog.h
	FODialog.cpp
	BMMDialog.cpp
	DirData.h
	widgets/DDListWidgets.h
	widgets/MultimediaWidget.cpp
	widgets/SlideshowWidget.cpp
	widgets/DirWidget2.cpp
	gitCompat.cpp
	gitWizard.cpp
	Browser.cpp
	BrowserWidget.cpp
	TrayUI.cpp
	OPWidget.cpp
	${TS_FILES}
	${QM_FILES}
)

add_custom_command(	
	TARGET lumina-fm
	COMMAND gzip -c ${CMAKE_CURRENT_SOURCE_DIR}/lumina-fm.1 > ${CMAKE_CURRENT_BINARY_DIR}/lumina-fm.1.gz
	OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/lumina-fm.1.gz
)

target_link_libraries(
    lumina-fm
    PRIVATE libLumina Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Multimedia Qt5::MultimediaWidgets Qt5::Network Qt5::Concurrent Qt5::Svg
)

install(TARGETS lumina-fm)
install(
	FILES ${QM_FILES}
	DESTINATION ${CMAKE_INSTALL_DATADIR}/lumina-desktop/i18n/
)
install(FILES lumina-fm.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(
	FILES ${CMAKE_CURRENT_BINARY_DIR}/lumina-fm.1.gz
	DESTINATION ${CMAKE_INSTALL_DATADIR}/man/man1
)
