SOURCES *= $${PWD}/LDesktopUtils.cpp
HEADERS *= $${PWD}/LDesktopUtils.h

INCLUDEPATH *= ${PWD}

#Now the other dependendies of it
include(LUtils.pri)
include(LuminaThemes.pri)
include(LuminaXDG.pri)
