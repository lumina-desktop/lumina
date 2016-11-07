QT *= multimedia svg

#LUtils Files
SOURCES *= $${PWD}/LuminaXDG.cpp
HEADERS *= $${PWD}/LuminaXDG.h

INCLUDEPATH *= ${PWD}

#include LUtils and LuminaOS
include(LUtils.pri)
