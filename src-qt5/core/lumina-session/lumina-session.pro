include($${PWD}/../../OS-detect.pri)

QT       = core widgets x11extras


TARGET = start-lumina-desktop
target.path = $${L_BINDIR}

include(../libLumina/LDesktopUtils.pri)
include(../libLumina/LuminaXDG.pri)
include(../libLumina/LuminaThemes.pri)

SOURCES += main.cpp \
			session.cpp

HEADERS  += session.h

INSTALLS += target
