
# Files
QT *= x11extras
LIBS *= -lc -lxcb -lxcb-ewmh -lxcb-icccm -lxcb-image -lxcb-composite -lxcb-damage -lxcb-util -lXdamage 

SOURCES *= $${PWD}/NativeWindow.cpp \
		$${PWD}/NativeWindowSystem.cpp \
		$${PWD}/NativeEventFilter.cpp

HEADERS *= $${PWD}/NativeWindow.h \
		$${PWD}/NativeWindowSystem.h \
		$${PWD}/NativeEventFilter.h

INCLUDEPATH *= $${PWD}
