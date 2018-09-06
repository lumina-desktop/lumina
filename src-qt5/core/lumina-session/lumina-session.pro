include($${PWD}/../../OS-detect.pri)

QT       = core widgets x11extras


TARGET = start-lumina-desktop
target.path = $${L_BINDIR}

include(../libLumina/LDesktopUtils.pri)
include(../libLumina/LuminaXDG.pri)
include(../libLumina/LuminaThemes.pri)
include(../libLumina/LuminaSingleApplication.pri)

SOURCES += main.cpp \
			session.cpp

HEADERS  += session.h

manpage.path=$${L_MANDIR}/man8/
manpage.extra="$${MAN_ZIP} $$PWD/start-lumina-desktop.8 > $(INSTALL_ROOT)$${L_MANDIR}/man8/start-lumina-desktop.8.gz"

INSTALLS += target manpage
