# Translation
set (TS_FILES
	i18n/lumina-config_af.ts
	i18n/lumina-config_ar.ts
	i18n/lumina-config_az.ts
	i18n/lumina-config_bg.ts
	i18n/lumina-config_bn.ts
	i18n/lumina-config_bs.ts
	i18n/lumina-config_ca.ts
	i18n/lumina-config_cs.ts
	i18n/lumina-config_cy.ts
	i18n/lumina-config_da.ts
	i18n/lumina-config_de.ts
	i18n/lumina-config_el.ts
	i18n/lumina-config_en_GB.ts
	i18n/lumina-config_en_ZA.ts
	i18n/lumina-config_en_AU.ts
	i18n/lumina-config_es.ts
	i18n/lumina-config_et.ts
	i18n/lumina-config_eu.ts
	i18n/lumina-config_fa.ts
	i18n/lumina-config_fi.ts
	i18n/lumina-config_fr.ts
	i18n/lumina-config_fr_CA.ts
	i18n/lumina-config_gl.ts
	i18n/lumina-config_he.ts
	i18n/lumina-config_hi.ts
	i18n/lumina-config_hr.ts
	i18n/lumina-config_hu.ts
	i18n/lumina-config_id.ts
	i18n/lumina-config_is.ts
	i18n/lumina-config_it.ts
	i18n/lumina-config_ja.ts
	i18n/lumina-config_ka.ts
	i18n/lumina-config_ko.ts
	i18n/lumina-config_lt.ts
	i18n/lumina-config_lv.ts
	i18n/lumina-config_mk.ts
	i18n/lumina-config_mn.ts
	i18n/lumina-config_ms.ts
	i18n/lumina-config_mt.ts
	i18n/lumina-config_nb.ts
	i18n/lumina-config_nl.ts
	i18n/lumina-config_pa.ts
	i18n/lumina-config_pl.ts
	i18n/lumina-config_pt.ts
	i18n/lumina-config_pt_BR.ts
	i18n/lumina-config_ro.ts
	i18n/lumina-config_ru.ts
	i18n/lumina-config_sk.ts
	i18n/lumina-config_sl.ts
	i18n/lumina-config_sr.ts
	i18n/lumina-config_sv.ts
	i18n/lumina-config_sw.ts
	i18n/lumina-config_ta.ts
	i18n/lumina-config_tg.ts
	i18n/lumina-config_th.ts
	i18n/lumina-config_tr.ts
	i18n/lumina-config_uk.ts
	i18n/lumina-config_uz.ts
	i18n/lumina-config_vi.ts
	i18n/lumina-config_zh_CN.ts
	i18n/lumina-config_zh_HK.ts
	i18n/lumina-config_zh_TW.ts
	i18n/lumina-config_zu.ts
)

qt_add_translation(QM_FILES ${TS_FILES})

add_executable(lumina-config 
	main.cpp
	mainWindow.cpp
	LPlugins.cpp
	ColorDialog.cpp
	ThemeDialog.cpp
	GetPluginDialog.cpp
	ScriptDialog.cpp
	PanelWidget.cpp
	AppDialog.h
	pages/PageWidget.h
	pages/getPage.cpp
	pages/page_main.cpp
	pages/page_wallpaper.cpp
#	pages/page_theme.cpp
	pages/page_autostart.cpp
	pages/page_defaultapps.cpp
	pages/page_fluxbox_keys.cpp
	pages/page_fluxbox_settings.cpp
	pages/page_interface_desktop.cpp
	pages/page_interface_menu.cpp
	pages/page_interface_panels.cpp
	pages/page_session_locale.cpp
	pages/page_session_options.cpp
	pages/page_compton.cpp  
	pages/page_soundtheme.cpp
#	pages/page_bluetooth_trueos.cpp 
#	pages/page_mouse_trueos.cpp 
	${TS_FILES}
	${QM_FILES}
)

target_link_libraries(
    lumina-config
    PRIVATE libLumina Qt5::Core Qt5::Gui Qt5::Widgets Qt5::X11Extras
)

add_custom_command(	
	TARGET lumina-config
	COMMAND gzip -c ${CMAKE_CURRENT_SOURCE_DIR}/lumina-config.1 > ${CMAKE_CURRENT_BINARY_DIR}/lumina-config.1.gz
	OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/lumina-config.1.gz
)

install(TARGETS lumina-config)
install(
	FILES ${QM_FILES}
	DESTINATION ${CMAKE_INSTALL_DATADIR}/lumina-desktop/i18n/
)
install(FILES lumina-config.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(
	FILES ${CMAKE_CURRENT_BINARY_DIR}/lumina-config.1.gz
	DESTINATION ${CMAKE_INSTALL_DATADIR}/man/man1
)
