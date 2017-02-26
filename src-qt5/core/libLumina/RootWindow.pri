
# Files
SOURCES *= $${PWD}/RootWindow.cpp \
		$${PWD}/RootSubWindow.cpp

HEADERS *= $${PWD}/RootWindow.h \
		$${PWD}/RootSubWindow.h

INCLUDEPATH *= ${PWD}

# include other library dependencies
include(LUtils.pri)
include(NativeWindow.pri);
