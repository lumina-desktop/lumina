
# Files
QT *= x11extras
LIBS *= -lc -lxcb -lxcb-ewmh -lxcb-icccm -lxcb-image -lxcb-composite -lxcb-damage -lxcb-util -lXdamage 

SOURCES *= $${PWD}/NativeWindow.cpp \
		$${PWD}/NativeWindowSystem.cpp

HEADERS *= $${PWD}/NativeWindow.h \
		$${PWD}/NativeWindowSystem.h

INCLUDEPATH *= $${PWD}
