QT *= multimedia

HEADERS *= $${PWD}/LVideoLabel.h
HEADERS *= $${PWD}/LVideoSurface.h
SOURCES *= $${PWD}/LVideoLabel.cpp
SOURCES *= $${PWD}/LVideoSurface.cpp

INCLUDEPATH *= ${PWD}

#Now the other dependendies of it
#include(LUtils.pri)
