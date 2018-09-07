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

TEMPLATE = app

SOURCES += main_panel.cpp \
	LPanel.cpp \
	AppMenu.cpp


HEADERS  += Globals.h \
	LPanel.h \
	LWinInfo.h \
	AppMenu.h \
	panel-plugins/LPPlugin.h \
	panel-plugins/NewPP.h \
	panel-plugins/LTBWidget.h \
	JsonMenu.h


#Now include all the files for the various plugins
include(panel-plugins/panel-plugins.pri)

INSTALLS += target
