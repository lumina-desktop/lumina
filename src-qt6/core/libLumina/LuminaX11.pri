
QT *= x11extras

LIBS *= -lc -lxcb -lxcb-ewmh -lxcb-icccm -lxcb-image -lxcb-composite -lxcb-damage -lxcb-util -lxcb-dpms -lXdamage -lX11

#LUtils Files
SOURCES *= $${PWD}/LuminaX11.cpp
HEADERS *= $${PWD}/LuminaX11.h

#X11 Bridge (Qt5/Qt6 abstraction layer)
SOURCES *= $${PWD}/LuminaX11Bridge.cpp
HEADERS *= $${PWD}/LuminaX11Bridge.h

INCLUDEPATH *= ${PWD}

#include LUtils and LuminaOS
include(LUtils.pri)
