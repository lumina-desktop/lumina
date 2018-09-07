include($${PWD}/../../OS-detect.pri)

QT       += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets x11extras multimedia concurrent svg



TARGET = lumina-panel
target.path = $${L_BINDIR}

#include all the special classes from the Lumina tree
include(../libLumina/ResizeMenu.pri)
include(../libLumina/LDesktopUtils.pri) #includes LUtils and LOS
include(../libLumina/LuminaXDG.pri)
include(../libLumina/LuminaX11.pri)
include(../libLumina/ExternalProcess.pri)
include(../libLumina/LIconCache.pri)
include(../libLumina/LuminaSingleApplication.pri)

TEMPLATE = app

SOURCES += main_panel.cpp \
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
include(desktop-plugins/desktop-plugins.pri)
include(panel-plugins/panel-plugins.pri)

INSTALLS += target
