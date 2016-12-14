
QT *= x11extras

LIBS *= -lc -lxcb -lxcb-ewmh -lxcb-icccm -lxcb-image -lxcb-composite -lxcb-damage -lxcb-util -lXdamage 

#LUtils Files
SOURCES *= $${PWD}/LuminaX11.cpp
HEADERS *= $${PWD}/LuminaX11.h

INCLUDEPATH *= ${PWD}

#include LUtils and LuminaOS
include(LUtils.pri)
