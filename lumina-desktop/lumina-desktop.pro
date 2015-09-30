
QT       += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets x11extras multimedia quickwidgets
CONFIG += debug_and_release

TARGET = Lumina-DE
isEmpty(PREFIX) {
 PREFIX = /usr/local
}
target.path = $$DESTDIR$$PREFIX/bin

isEmpty(LIBPREFIX) {
 LIBPREFIX = $$PREFIX/lib
}

LIBS     += -L../libLumina -L$$LIBPREFIX -lLuminaUtils -lxcb -lxcb-damage
QMAKE_LIBDIR	= ../libLumina
DEPENDPATH	+= ../libLumina

TEMPLATE = app

isEmpty(QT5LIBDIR) {
 QT5LIBDIR = $$PREFIX/lib/qt5
}

LRELEASE = $$QT5LIBDIR/bin/lrelease


SOURCES += main.cpp \
	WMProcess.cpp \
	LXcbEventFilter.cpp \
	LSession.cpp \
	LDesktop.cpp \
	LDesktopPluginSpace.cpp \
	LPanel.cpp \
	LWinInfo.cpp \
	AppMenu.cpp \
	SettingsMenu.cpp \
	SystemWindow.cpp \
	BootSplash.cpp \
	panel-plugins/userbutton/LUserButton.cpp \
	panel-plugins/userbutton/UserWidget.cpp \
	panel-plugins/userbutton/UserItemWidget.cpp \
	panel-plugins/desktopbar/LDeskBar.cpp \
	panel-plugins/taskmanager/LTaskManagerPlugin.cpp \
	panel-plugins/taskmanager/LTaskButton.cpp \
	panel-plugins/systemtray/LSysTray.cpp \
	panel-plugins/systemtray/TrayIcon.cpp \
	panel-plugins/clock/LClock.cpp \
	panel-plugins/battery/LBattery.cpp \
	panel-plugins/desktopswitcher/LDesktopSwitcher.cpp \
	panel-plugins/systemdashboard/LSysDashboard.cpp \
	panel-plugins/systemdashboard/SysMenuQuick.cpp \
	panel-plugins/showdesktop/LHomeButton.cpp \
	panel-plugins/appmenu/LAppMenuPlugin.cpp \
	panel-plugins/applauncher/AppLaunchButton.cpp \
	panel-plugins/systemstart/LStartButton.cpp \
	panel-plugins/systemstart/StartMenu.cpp \
	panel-plugins/systemstart/ItemWidget.cpp \
	desktop-plugins/LDPlugin.cpp \
	desktop-plugins/applauncher/AppLauncherPlugin.cpp \
	desktop-plugins/desktopview/DesktopViewPlugin.cpp \
	desktop-plugins/notepad/NotepadPlugin.cpp \
	desktop-plugins/audioplayer/PlayerWidget.cpp \
	desktop-plugins/systemmonitor/MonitorWidget.cpp
#	desktop-plugins/messagecenter/MessageCenter.cpp


HEADERS  += Globals.h \
	WMProcess.h \
	LXcbEventFilter.h \
	LSession.h \
	LDesktop.h \
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
	desktop-plugins/LDPluginContainer.h \
	desktop-plugins/NewDP.h \
	panel-plugins/userbutton/LUserButton.h \
	panel-plugins/userbutton/UserWidget.h \
	panel-plugins/userbutton/UserItemWidget.h \
	panel-plugins/desktopbar/LDeskBar.h \
	panel-plugins/systemtray/LSysTray.h \
	panel-plugins/systemtray/TrayIcon.h \
	panel-plugins/spacer/LSpacer.h \
	panel-plugins/clock/LClock.h \
	panel-plugins/battery/LBattery.h \
	panel-plugins/desktopswitcher/LDesktopSwitcher.h \
	panel-plugins/taskmanager/LTaskManagerPlugin.h \
	panel-plugins/taskmanager/LTaskButton.h \
	panel-plugins/systemdashboard/LSysDashboard.h \
	panel-plugins/systemdashboard/SysMenuQuick.h \
	panel-plugins/showdesktop/LHomeButton.h \
	panel-plugins/appmenu/LAppMenuPlugin.h \
	panel-plugins/applauncher/AppLaunchButton.h \
	panel-plugins/systemstart/LStartButton.h \
	panel-plugins/systemstart/StartMenu.h \
	panel-plugins/systemstart/ItemWidget.h \
#	panel-plugins/quickcontainer/QuickPPlugin.h \
	desktop-plugins/SamplePlugin.h \
	desktop-plugins/calendar/CalendarPlugin.h \
	desktop-plugins/applauncher/AppLauncherPlugin.h \
	desktop-plugins/desktopview/DesktopViewPlugin.h \
	desktop-plugins/notepad/NotepadPlugin.h \
	desktop-plugins/audioplayer/PlayerWidget.h \
	desktop-plugins/systemmonitor/MonitorWidget.h \
	desktop-plugins/quickcontainer/QuickDPlugin.h
#	desktop-plugins/messagecenter/MessageCenter.h

FORMS    += SystemWindow.ui \
	BootSplash.ui \
	panel-plugins/userbutton/UserWidget.ui \
	panel-plugins/systemdashboard/SysMenuQuick.ui \
	panel-plugins/systemstart/StartMenu.ui \
	desktop-plugins/audioplayer/PlayerWidget.ui \
	desktop-plugins/systemmonitor/MonitorWidget.ui

RESOURCES+= Lumina-DE.qrc

INCLUDEPATH += ../libLumina $$PREFIX/include

desktop.files = Lumina-DE.desktop
linux-* {
desktop.path = /usr/share/xsessions
} else {
desktop.path = $$PREFIX/share/xsessions
}

icons.files = Lumina-DE.png \
		Insight-FileManager.png

icons.path = $$PREFIX/share/pixmaps

wallpapers.files = wallpapers/Lumina_Wispy_gold_1920x1080.jpg \
			wallpapers/Lumina_Wispy_green_1920x1080.jpg \
			wallpapers/Lumina_Wispy_purple_1920x1080.jpg \
			wallpapers/Lumina_Wispy_red_1920x1080.jpg
wallpapers.path = $$PREFIX/share/wallpapers/Lumina-DE

fluxconf.files = fluxboxconf/fluxbox-init-rc \
			fluxboxconf/fluxbox-keys
fluxconf.path = $$PREFIX/share/Lumina-DE/

defaults.files = defaults/desktop-background.jpg \
		defaults/defaultapps.conf \
		defaults/luminaDesktop.conf \
		audiofiles/Logout.ogg \
		audiofiles/Login.ogg
defaults.path = $$PREFIX/share/Lumina-DE/

conf.path = $$PREFIX/etc
conf.extra = cp defaults/luminaDesktop.conf $(INSTALL_ROOT)$$PREFIX/etc/luminaDesktop.conf.dist

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

dotrans.path=$$PREFIX/share/Lumina-DE/i18n/
dotrans.extra=cd i18n && $${LRELEASE} -nounfinished *.ts && cp *.qm $(INSTALL_ROOT)$$PREFIX/share/Lumina-DE/i18n/

INSTALLS += target desktop icons wallpapers defaults conf fluxconf dotrans

NO_I18N{
  INSTALLS -= dotrans
}