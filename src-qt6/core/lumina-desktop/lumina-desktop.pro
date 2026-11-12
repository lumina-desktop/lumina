include($${PWD}/../../OS-detect.pri)

lessThan(QT_MAJOR_VERSION, 5) {
  message("[ERROR] Qt 5.11+ is required to use the Lumina Desktop!")
  exit
}
lessThan(QT_MINOR_VERSION, 11){
  message("[ERROR] Qt 5.11+ is required to use the Lumina Desktop!")
  exit
}
QT       += core gui network widgets x11extras multimedia multimediawidgets concurrent svg quick qml


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
include(../libLumina/LIconCache.pri)
include(../libLumina/DesktopSettings.pri)

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
	global-includes.h \
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


#include the individual desktop components
include(../../src-cpp/plugins-base.pri)
include(src-screensaver/screensaver.pri)

#Now include all the files for the various plugins
include(panel-plugins/panel-plugins.pri)
include(desktop-plugins/desktop-plugins.pri)

RESOURCES+= Lumina-DE.qrc

desktop.path = $${L_SESSDIR}
desktop.files = Lumina-DE.desktop

icons.files = Lumina-DE.svg

fluxconf.files = fluxboxconf/fluxbox-init-rc \
			fluxboxconf/fluxbox-keys
fluxconf.path = $${L_SHAREDIR}/lumina-desktop/

fluxtheme.files = fluxboxconf/theme.cfg
fluxtheme.path = $${L_SHAREDIR}/lumina-desktop/ 

defaults.files = defaults/luminaDesktop.conf \
		defaults/compton.conf \
		audiofiles/Logout.ogg \
		audiofiles/Login.ogg \
		audiofiles/low-battery.ogg
defaults.path = $${L_SHAREDIR}/lumina-desktop/

conf.path = $${L_ETCDIR}

extrafiles.path = $${L_SHAREDIR}/lumina-desktop
extrafiles.files = extrafiles/*

#Now do any OS-specific defaults (if available)
#First see if there is a known OS override first
!isEmpty(DEFAULT_SETTINGS){ 
  message("Installing defaults settings for OS: $${DEFAULT_SETTINGS}")
  OS=$${DEFAULT_SETTINGS}
}
exists("$$PWD/defaults/luminaDesktop-$${OS}.conf"){
  message(" -- Found OS-specific system config file: $${OS}");
  conf.extra = cp $$PWD/defaults/luminaDesktop-$${OS}.conf $(INSTALL_ROOT)$${L_ETCDIR}/luminaDesktop.conf.dist
}else{
  conf.extra = cp $$PWD/defaults/luminaDesktop.conf $(INSTALL_ROOT)$${L_ETCDIR}/luminaDesktop.conf.dist
}
exists("$$PWD/defaults/desktop-background-$${OS}.jpg"){
  message(" -- Found OS-specific background image: $${OS}");
  defaults.extra = cp $$PWD/defaults/desktop-background-$${OS}.jpg $(INSTALL_ROOT)$${L_SHAREDIR}/lumina-desktop/desktop-background.jpg
}else{
  defaults.extra = cp $$PWD/defaults/desktop-background.jpg $(INSTALL_ROOT)$${L_SHAREDIR}/lumina-desktop/desktop-background.jpg
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
                i18n/lumina-desktop_en_AU.ts \
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
dotrans.extra=cd $$PWD/i18n && $${LRELEASE} -nounfinished *.ts && cp *.qm $(INSTALL_ROOT)$${L_SHAREDIR}/lumina-desktop/i18n/

manpage.path=$${L_MANDIR}/man1/
manpage.extra="$${MAN_ZIP} $$PWD/lumina-desktop.1 > $(INSTALL_ROOT)$${L_MANDIR}/man1/lumina-desktop.1.gz"

INSTALLS += target desktop icons defaults conf fluxconf fluxtheme manpage extrafiles

WITH_I18N{
  INSTALLS += dotrans
}
