include("$${PWD}/../../../OS-detect.pri")

QT += core widgets x11extras
TARGET = lcc-menu
target.path = $${L_BINDIR}

TEMPLATE = app

#include all the special classes from the Lumina tree
include(../../../core/libLumina/LDesktopUtils.pri) #includes LUtils
include(../../../core/libLumina/LuminaXDG.pri)
include(../../../core/libLumina/LuminaSingleApplication.pri)
include(../../../core/libLumina/LuminaThemes.pri)

SOURCES += main.cpp \
           MainWindow.cpp \
           LPlugins.cpp \
           GetPluginDialog.cpp \
           AppDialog.cpp \
           ScriptDialog.cpp

HEADERS += MainWindow.h \
           LPlugins.h \
           GetPluginDialog.h \
           AppDialog.h \
           ScriptDialog.h

FORMS += MainWindow.ui \
         GetPluginDialog.ui \
         AppDialog.ui \
         ScriptDialog.ui

desktop.files=lcc-menu.desktop
desktop.path=$${L_SHAREDIR}/applications/

INSTALLS += target desktop
