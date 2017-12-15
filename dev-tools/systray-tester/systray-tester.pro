TEMPLATE	= app
LANGUAGE	= C++
QT += core gui widgets
CONFIG	+= qt warn_on release

include(../../src-qt5/core/libLumina/LuminaXDG.pri)

HEADERS	+= Trayapp.h

SOURCES	+= main.cpp

INSTALLS =

TARGET  = test-tray
