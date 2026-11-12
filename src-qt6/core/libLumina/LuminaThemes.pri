include("$${PWD}/../../OS-detect.pri")

#LUtils Files
SOURCES *= $${PWD}/LuminaThemes.cpp
HEADERS *= $${PWD}/LuminaThemes.h

INCLUDEPATH *= ${PWD}

#include LUtils and LuminaOS
include(LUtils.pri)
