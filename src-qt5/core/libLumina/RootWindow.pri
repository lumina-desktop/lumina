
# Files
SOURCES *= $${PWD}/RootWindow.cpp \
		$${PWD}/RootSubWindow.cpp

HEADERS *= $${PWD}/RootWindow.h \
		$${PWD}/RootSubWindow.h

INCLUDEPATH *= ${PWD}

# include LUtils and LuminaX11
include(LUtils.pri)
