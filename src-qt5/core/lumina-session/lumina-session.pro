include($${PWD}/../../OS-detect.pri)

QT       += core widgets x11extras


TARGET = start-lumina-desktop
target.path = $${L_BINDIR}


LIBS     += -lLuminaUtils -lxcb -lxcb-damage
DEPENDPATH	+= ../libLumina

SOURCES += main.cpp

HEADERS  += session.h

INSTALLS += target
