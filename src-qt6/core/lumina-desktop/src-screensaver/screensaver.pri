SOURCES *= $${PWD}/LLockScreen.cpp \
	$${PWD}/LScreenSaver.cpp \
	$${PWD}/SSBaseWidget.cpp

HEADERS *= $${PWD}/LLockScreen.h \
	$${PWD}/LScreenSaver.h \
	$${PWD}/SSBaseWidget.h

FORMS *= $${PWD}/LLockScreen.ui

#update the includepath so we can just (#include <LScreenSaver.h>) as needed without paths
INCLUDEPATH *= $${PWD}

#Now include all the screensaver animations/options
#include(animations/animations.pri)
