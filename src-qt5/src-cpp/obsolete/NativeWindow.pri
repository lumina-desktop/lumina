
# Files
QT *= x11extras
LIBS *= -lc -lxcb -lxcb-ewmh -lxcb-icccm -lxcb-image -lxcb-composite -lxcb-damage -lxcb-util -lxcb-keysyms -lXdamage 

SOURCES *= $${PWD}/NativeWindowSystem.cpp \
		$${PWD}/NativeKeyToQt.cpp \
		$${PWD}/NativeEventFilter.cpp

HEADERS *= $${PWD}/NativeWindowSystem.h \
		$${PWD}/NativeEventFilter.h

INCLUDEPATH *= $${PWD}
