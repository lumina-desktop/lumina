#include("$${PWD}/../../OS-detect.pri")

QT *= x11extras

#X11/XCB includes
LIBS *= -lxcb -lxcb-randr

SOURCES *= $${PWD}/LuminaRandR-X11.cpp

#General API/Header
HEADERS *= $${PWD}/LuminaRandR.h

INCLUDEPATH *= ${PWD}
