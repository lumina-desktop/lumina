include("$${PWD}/../../OS-detect.pri")

QT *= network x11extras

#LUtils Files
SOURCES *= $${PWD}/LuminaSingleApplication.cpp
HEADERS *= $${PWD}/LuminaSingleApplication.h

INCLUDEPATH *= ${PWD}

#include LUtils and LuminaOS
include(LUtils.pri)
