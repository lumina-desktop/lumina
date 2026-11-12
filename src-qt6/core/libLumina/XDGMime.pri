#QT *= multimedia svg

#XDG Mime Files
SOURCES *= $${PWD}/XDGMime.cpp
HEADERS *= $${PWD}/XDGMime.h

INCLUDEPATH *= ${PWD}

#include LUtils and LuminaOS
include(LUtils.pri)
