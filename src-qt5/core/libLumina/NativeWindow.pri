
# Files
QT *= x11extras
LIBS *= -lc -lxcb -lxcb-ewmh -lxcb-icccm -lxcb-image -lxcb-composite -lxcb-damage -lxcb-util -lxcb-keysyms -lXdamage 
#QT *= -lxcb-render -lxcb-render-util

SOURCES *= $${PWD}/NativeWindow.cpp \
		$${PWD}/NativeWindowSystem.cpp \
		$${PWD}/NativeKeyToQt.cpp \
		$${PWD}/NativeEventFilter.cpp \
		$${PWD}/NativeEmbedWidget.cpp

HEADERS *= $${PWD}/NativeWindow.h \
		$${PWD}/NativeWindowSystem.h \
		$${PWD}/NativeEventFilter.h \
		$${PWD}/NativeEmbedWidget.h

INCLUDEPATH *= $${PWD}
