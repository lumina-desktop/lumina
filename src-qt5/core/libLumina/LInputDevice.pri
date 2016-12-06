
QT *= x11extras

LIBS *= -lc -lxcb -lxcb-xinput

#LUtils Files
SOURCES *= $${PWD}/LInputDevice.cpp
HEADERS *= $${PWD}/LInputDevice.h

INCLUDEPATH *= ${PWD}

#include LUtils and LuminaOS
include(LUtils.pri)
