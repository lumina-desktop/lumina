
QT *= gui qml quick

SOURCES *= $${PWD}/RootDesktopObject.cpp \
		$${PWD}/ScreenObject.cpp

HEADERS *= $${PWD}/RootDesktopObject.h \
		$${PWD}/ScreenObject.h

INCLUDEPATH *= $${PWD}

include($${PWD}/../src-qml/src-qml.pri)
