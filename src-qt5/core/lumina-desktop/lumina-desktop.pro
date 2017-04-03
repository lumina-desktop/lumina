include($${PWD}/../../OS-detect.pri)

QT       += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets x11extras multimedia concurrent svg



TARGET = lumina-desktop
target.path = $${L_BINDIR}

#include all the special classes from the Lumina tree
include(../libLumina/ResizeMenu.pri)
include(../libLumina/LDesktopUtils.pri) #includes LUtils and LOS
include(../libLumina/LuminaXDG.pri)
include(../libLumina/LuminaX11.pri)
include(../libLumina/LuminaSingleApplication.pri)
include(../libLumina/LuminaThemes.pri)
include(../libLumina/ExternalProcess.pri)

TEMPLATE = app

SOURCES += main.cpp \
	WMProcess.cpp \
	LXcbEventFilter.cpp \
	LSession.cpp \
	LDesktop.cpp \
	LDesktopBackground.cpp \
	LDesktopPluginSpace.cpp \
	LPanel.cpp \
	LWinInfo.cpp \
	AppMenu.cpp \
	SettingsMenu.cpp \
	SystemWindow.cpp \
	BootSplash.cpp \
	desktop-plugins/LDPlugin.cpp


HEADERS  += Globals.h \
	WMProcess.h \
	LXcbEventFilter.h \
	LSession.h \
	LDesktop.h \
	LDesktopBackground.h \
	LDesktopPluginSpace.h \
	LPanel.h \
	LWinInfo.h \
	AppMenu.h \
	SettingsMenu.h \
	SystemWindow.h \
	BootSplash.h \
	panel-plugins/LPPlugin.h \
	panel-plugins/NewPP.h \
	panel-plugins/LTBWidget.h \
	desktop-plugins/LDPlugin.h \
	desktop-plugins/NewDP.h \
	JsonMenu.h

FORMS    += SystemWindow.ui \
	BootSplash.ui 


#Now include all the files for the various plugins
include(panel-plugins/panel-plugins.pri)
include(desktop-plugins/desktop-plugins.pri)

RESOURCES+= Lumina-DE.qrc

desktop.path = $${L_SESSDIR}
desktop.files = Lumina-DE.desktop

icons.files = Lumina-DE.png \
		Insight-FileManager.png
icons.path = $${L_SHAREDIR}/pixmaps

fluxconf.files = fluxboxconf/fluxbox-init-rc \
			fluxboxconf/fluxbox-keys
fluxconf.path = $${L_SHAREDIR}/lumina-desktop/

wallpapers.files = wallpapers/Lumina_Wispy_gold.jpg \
			wallpapers/Lumina_Wispy_green.jpg \
			wallpapers/Lumina_Wispy_purple.jpg \
			wallpapers/Lumina_Wispy_red.jpg \
			wallpapers/Lumina_Wispy_blue-grey.jpg \
			wallpapers/Lumina_Wispy_blue-grey-zoom.jpg \
			wallpapers/Lumina_Wispy_grey-blue.jpg \
			wallpapers/Lumina_Wispy_grey-blue-zoom.jpg 
wallpapers.path = $${L_SHAREDIR}/wallpapers/Lumina-DE


defaults.files = defaults/luminaDesktop.conf \
		defaults/compton.conf \
		audiofiles/Logout.ogg \
		audiofiles/Login.ogg \
		audiofiles/low-battery.ogg
defaults.path = $${L_SHAREDIR}/lumina-desktop/

conf.path = $${L_ETCDIR}

#Now do any OS-specific defaults (if available)
#First see if there is a known OS override first
!isEmpty(DEFAULT_SETTINGS){ 
  message("Installing defaults settings for OS: $${DEFAULT_SETTINGS}")
  OS=$${DEFAULT_SETTINGS}
}
exists("defaults/luminaDesktop-$${OS}.conf"){
  message(" -- Found OS-specific system config file: $${OS}");
  conf.extra = cp defaults/luminaDesktop-$${OS}.conf $(INSTALL_ROOT)$${L_ETCDIR}/luminaDesktop.conf.dist
}else{
  conf.extra = cp defaults/luminaDesktop.conf $(INSTALL_ROOT)$${L_ETCDIR}/luminaDesktop.conf.dist
}
exists("defaults/desktop-background-$${OS}.jpg"){
  message(" -- Found OS-specific background image: $${OS}");
  defaults.extra = cp defaults/desktop-background-$${OS}.jpg $(INSTALL_ROOT)$${L_SHAREDIR}/lumina-desktop/desktop-background.jpg
}else{
  defaults.extra = cp defaults/desktop-background.jpg $(INSTALL_ROOT)$${L_SHAREDIR}/lumina-desktop/desktop-background.jpg
}

TRANSLATIONS =  i18n/lumina-desktop_af.ts \
                i18n/lumina-desktop_ar.ts \
                i18n/lumina-desktop_az.ts \
                i18n/lumina-desktop_bg.ts \
                i18n/lumina-desktop_bn.ts \
                i18n/lumina-desktop_bs.ts \
                i18n/lumina-desktop_ca.ts \
                i18n/lumina-desktop_cs.ts \
                i18n/lumina-desktop_cy.ts \
                i18n/lumina-desktop_da.ts \
                i18n/lumina-desktop_de.ts \
                i18n/lumina-desktop_el.ts \
                i18n/lumina-desktop_en_GB.ts \
                i18n/lumina-desktop_en_ZA.ts \
                i18n/lumina-desktop_es.ts \
                i18n/lumina-desktop_et.ts \
                i18n/lumina-desktop_eu.ts \
                i18n/lumina-desktop_fa.ts \
                i18n/lumina-desktop_fi.ts \
                i18n/lumina-desktop_fr.ts \
                i18n/lumina-desktop_fr_CA.ts \
                i18n/lumina-desktop_gl.ts \
                i18n/lumina-desktop_he.ts \
                i18n/lumina-desktop_hi.ts \
                i18n/lumina-desktop_hr.ts \
                i18n/lumina-desktop_hu.ts \
                i18n/lumina-desktop_id.ts \
                i18n/lumina-desktop_is.ts \
                i18n/lumina-desktop_it.ts \
                i18n/lumina-desktop_ja.ts \
                i18n/lumina-desktop_ka.ts \
                i18n/lumina-desktop_ko.ts \
                i18n/lumina-desktop_lt.ts \
                i18n/lumina-desktop_lv.ts \
                i18n/lumina-desktop_mk.ts \
                i18n/lumina-desktop_mn.ts \
                i18n/lumina-desktop_ms.ts \
                i18n/lumina-desktop_mt.ts \
                i18n/lumina-desktop_nb.ts \
                i18n/lumina-desktop_nl.ts \
                i18n/lumina-desktop_pa.ts \
                i18n/lumina-desktop_pl.ts \
                i18n/lumina-desktop_pt.ts \
                i18n/lumina-desktop_pt_BR.ts \
                i18n/lumina-desktop_ro.ts \
                i18n/lumina-desktop_ru.ts \
                i18n/lumina-desktop_sk.ts \
                i18n/lumina-desktop_sl.ts \
                i18n/lumina-desktop_sr.ts \
                i18n/lumina-desktop_sv.ts \
                i18n/lumina-desktop_sw.ts \
                i18n/lumina-desktop_ta.ts \
                i18n/lumina-desktop_tg.ts \
                i18n/lumina-desktop_th.ts \
                i18n/lumina-desktop_tr.ts \
                i18n/lumina-desktop_uk.ts \
                i18n/lumina-desktop_uz.ts \
                i18n/lumina-desktop_vi.ts \
                i18n/lumina-desktop_zh_CN.ts \
                i18n/lumina-desktop_zh_HK.ts \
                i18n/lumina-desktop_zh_TW.ts \
                i18n/lumina-desktop_zu.ts

dotrans.path=$${L_SHAREDIR}/lumina-desktop/i18n/
dotrans.extra=cd i18n && $${LRELEASE} -nounfinished *.ts && cp *.qm $(INSTALL_ROOT)$${L_SHAREDIR}/lumina-desktop/i18n/

manpage.path=$${L_MANDIR}/man8/
manpage.extra="$${MAN_ZIP} lumina-desktop.8 > $(INSTALL_ROOT)$${L_MANDIR}/man8/lumina-desktop.8.gz"

INSTALLS += target desktop icons wallpapers defaults conf fluxconf manpage

WITH_I18N{
  INSTALLS += dotrans
}
