# Files
QT *= x11extras
LIBS *= -lc -lxcb -lxcb-ewmh -lxcb-icccm -lxcb-image -lxcb-composite -lxcb-damage -lxcb-util -lxcb-keysyms -lxcb-screensaver -lXdamage 

SOURCES *= $${PWD}/LShortcutEvents.cpp \
		$${PWD}/NativeEventFilter.cpp \
		$${PWD}/NativeKeyToQt.cpp \
		$${PWD}/NativeWindowSystem.cpp \
		$${PWD}/Window-mgmt.cpp


HEADERS *= $${PWD}/LShortcutEvents.h \
		$${PWD}/NativeEventFilter.h \
		$${PWD}/NativeWindowSystem.h

#update the includepath so we can just (#include <NativeEventFilter.h>) as needed without paths
INCLUDEPATH *= ${PWD}
