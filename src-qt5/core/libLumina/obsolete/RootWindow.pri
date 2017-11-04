
# Files
SOURCES *= $${PWD}/RootWindow.cpp \
		$${PWD}/RootWindow-mgmt.cpp \
		$${PWD}/RootSubWindow.cpp \
		$${PWD}/RootSubWindow-animations.cpp

HEADERS *= $${PWD}/RootWindow.h \
		$${PWD}/RootSubWindow.h

INCLUDEPATH *= ${PWD}

# include other library dependencies
include(LUtils.pri)
include(NativeWindow.pri)
include(LIconCache.pri)
include(DesktopSettings.pri)
