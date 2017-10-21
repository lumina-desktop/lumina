QT *= multimedia

HEADERS *= $${PWD}/LVideoLabel.h
HEADERS *= $${PWD}/LVideoSurface.h
HEADERS *= $${PWD}/LVideoWidget.h
SOURCES *= $${PWD}/LVideoLabel.cpp
SOURCES *= $${PWD}/LVideoSurface.cpp
SOURCES *= $${PWD}/LVideoWidget.cpp

INCLUDEPATH *= ${PWD}

#Now the other dependendies of it
#include(LUtils.pri)
