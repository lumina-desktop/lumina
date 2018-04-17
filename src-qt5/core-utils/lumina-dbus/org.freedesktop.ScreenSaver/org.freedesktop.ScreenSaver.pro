include("$${PWD}/../../../OS-detect.pri")

TARGET = lumina-org.freedesktop.ScreenSaver
TEMPLATE = app

QT += core dbus

# Needed for singleapplication
QT += gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
#QT -= gui
#CONFIG += console

CONFIG -= app_bundle

SOURCES += main.cpp
HEADERS += screensaver.h

#include special classes from the Lumina tree
include(../../../core/libLumina/LDesktopUtils.pri) #includes LUtils
include(../../../core/libLumina/LuminaXDG.pri)
include(../../../core/libLumina/LuminaSingleApplication.pri)

target.path = $${L_BINDIR}
target_desktop.path = $${L_ETCDIR}/xdg/autostart
target_desktop.files = lumina-freedesktop-screensaver.desktop
INSTALLS += target target_desktop
