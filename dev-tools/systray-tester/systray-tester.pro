TEMPLATE	= app
LANGUAGE	= C++
QT += core gui widgets
CONFIG	+= qt warn_on release

LIBS	+= -L../../libLumina -L/usr/local/lib -lLuminaUtils

HEADERS	+= Trayapp.h

SOURCES	+= main.cpp

INSTALLS =

QMAKE_LIBDIR = /usr/local/lib/qt5

TARGET  = test-tray

INCLUDEPATH+= ../../libLumina /usr/local/include
