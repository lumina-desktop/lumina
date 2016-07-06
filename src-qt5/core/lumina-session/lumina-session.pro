include($${PWD}/../../OS-detect.pri)

QT       = core widgets


TARGET = start-lumina-desktop
target.path = $${L_BINDIR}


LIBS     += -lLuminaUtils
DEPENDPATH	+= ../libLumina

SOURCES += main.cpp \
			session.cpp

HEADERS  += session.h

INSTALLS += target
