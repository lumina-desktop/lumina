
QT *= x11extras

LIBS *= -lc -lxcb -lxcb-ewmh -lxcb-icccm -lxcb-image -lxcb-composite -lxcb-damage -lxcb-util -lxcb-dpms -lXdamage -lX11

#LUtils Files
SOURCES *= $${PWD}/LuminaX11.cpp
HEADERS *= $${PWD}/LuminaX11.h

INCLUDEPATH *= ${PWD}

#include LUtils and LuminaOS
include(LUtils.pri)
