QT *= multimedia svg

#LUtils Files
SOURCES *= $${PWD}/LuminaXDG.cpp \
		$${PWD}/LFileInfo.cpp
HEADERS *= $${PWD}/LuminaXDG.h \
		$${PWD}/LFileInfo.h

INCLUDEPATH *= $${PWD}

#include LUtils and LuminaOS
include(LUtils.pri)
